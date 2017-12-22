/*
 * This file is part of GreatFET
 */

#include <stdint.h>
#include <stdbool.h>
#include <string.h>

#include "usb.h"
#include "usb_host.h"
#include "usb_type.h"
#include "usb_queue_host.h"
#include "usb_registers.h"
#include "usb_standard_request.h"
#include "greatfet_core.h"

#include "pins.h"

#include <libopencm3/lpc43xx/creg.h>
#include <libopencm3/lpc43xx/m4/nvic.h>
#include <libopencm3/lpc43xx/rgu.h>
#include <libopencm3/lpc43xx/usb.h>
#include <libopencm3/lpc43xx/scu.h>

static void usb_host_isr(usb_peripheral_t *host);

// Thunks to allow injection of USB0/USB1.
static void usb_host_isr_usb0(void) { usb_host_isr(&usb_peripherals[0]); }
static void usb_host_isr_usb1(void) { usb_host_isr(&usb_peripherals[1]); }

// Look up table to find our thunks easily.
static const vector_table_entry_t usb_peripheral_isrs[] = { usb_host_isr_usb0, usb_host_isr_usb1 };

/**
 * Enable sourcing a given USB port's VBUS, if possible.
 *
 * TODO: Support other methods of enabling/disabling VBUS beyond the GF1 load switch?
 */
int usb_provide_vbus(usb_peripheral_t *host)
{

	if(host->controller == 0) {

		// Enable the port power bit.
		USB_REG(host->controller)->PORTSC1 |= USB0_PORTSC1_H_PP;

		// TODO: handle supply of USB1 VBUS, where possible:
	} else {

		// Enable the port power bit.
		USB_REG(host->controller)->PORTSC1 |= USB1_PORTSC1_H_PP;

#ifdef BOARD_CAPABILITY_USB1_PROVIDE_VBUS
#ifdef BOARD_CAPABILITY_USB1_SENSE_VBUS

		// Disable the load switch, to ensure we're not already providing VBUS.
		gpio_clear(&gpio_usb1_en);
		
		// TODO: Do we need to delay, here?

		// If we can sense VBUS and we're not trying to drive it,
		// something else is driving VBUS. We don't want to turn VBUS on.
		if(gpio_read(&gpio_usb1_sense)) {
			return -1; // TODO: error code?
		}

#endif

		// Enable the load switch, providing VBUS.
		gpio_set(&gpio_usb1_en);

#endif
	}

	return 0;
}

/**
 * Disable a given USB port's VBUS, if possible.
 */
void usb_stop_providing_vbus(usb_peripheral_t *host)
{
	if(host->controller == 0) {
		// TODO: handle supply of USB1 VBUS, where possible:
	} else {

#ifdef BOARD_CAPABILITY_USB1_PROVIDE_VBUS
		// Disable the load switch.
		gpio_clear(&gpio_usb1_en);
#endif
		}
}




/**
 * Enable pull-down resistors on DM/DP, as required in host mode.
 */
void usb_host_enable_pulldowns(usb_peripheral_t *host)
{
#ifdef BOARD_QUIRK_USE_INTERNAL_DM_PULLDOWN
	// If this board doesn't explicitly provide a pull-down resistor on DM,
	// we can use the OTG termination resistor as a pull-down. Turn it on.
	USB_REG(host->controller)->OTGSC |= USB0_OTGSC_OT;
#endif

	// FIXME: The GreatFET one currently has no pull-down resistor on DP.
	// This isn't great, but we can potentially work without it, as the pin
	// tends to float down.

}


/**
 * Disable pull-down resistors on DM/DP, e.g. when switching out of host mode.
 */
void usb_host_disable_pulldowns(usb_peripheral_t *host)
{
#ifdef BOARD_QUIRK_USE_INTERNAL_DM_PULLDOWN
	// If this board doesn't explicitly provide a pull-down resistor on DM,
	// we use the OTG termination resistor as a pull-down. Turn it off.
		USB_REG(host->controller)->OTGSC &= ~USB0_OTGSC_OT;
#endif

	// FIXME: The GreatFET one currently has no pull-down resistor on DP.
	// This isn't great, but we can potentially work without it, as the pin
	// tends to float down.
}





/**
 * Places the given USB peripheral into host mode.
 */
static void usb_controller_set_host_mode(usb_peripheral_t *host)
{
	int host_number = host->controller;

	// Clear the RunStop bit to ensure the host is off, and wait for any
	// errors that exist to clear.
	USB_REG(host_number)->USBCMD &= ~USB0_USBCMD_H_RS;
	while(USB_REG(host_number)->USBSTS & USB0_USBSTS_H_HCH);

	// Place the USB controller in host mode.
	USB_REG(host_number)->USBMODE &= ~(USB0_USBMODE_H_CM_MASK);
	USB_REG(host_number)->USBMODE |= USB0_USBMODE_H_CM(USBMODE_HOST_MODE);

	// Enable the required pull-down resistors.
	usb_host_enable_pulldowns(host);

	// Mark this device as in host mode.
	host->mode = USB_CONTROLLER_MODE_HOST;
}


/**
 * Configures the USB host to support the interrupts we use for host mode.
 *
 * Installs an interrupt handler for USB events. If desired, this can
 * be replaced afterwards with usb_set_irq_handler.
 */
static void usb_controller_set_up_host_interrupts(usb_peripheral_t *host)
{
	int host_number = host->controller;

	// Disable all currently active interrupts for the given controller.
	// TODO: Do we want to clear all interrupts, here?
	USB_REG(host_number)->USBINTR = 0;
	USB_REG(host_number)->USBSTS = ~0;

	// Enable the interrupt modes we want by default for this controller.
	USB_REG(host_number)->USBINTR |=
		USB0_USBINTR_H_UEE	| // USB Error
		USB0_USBINTR_H_PCE	| // Port Change
		USB0_USBINTR_H_AAE	| // Asynch Queue Advance
		USB0_USBINTR_H_UAIE | // transaction from Async queue finished
		USB0_USBINTR_H_UPIA;  // transaction from the Periodc queue finished

	// Set up a default handler for USB events.
	usb_set_irq_handler(host, usb_peripheral_isrs[host_number]);
}


/**
 * Set up the linked list used to queue up asynchronous transfers.
 */
static void usb_controller_set_up_async_list(usb_peripheral_t *host)
{
	// Clear the structure to its all-zeroes state.
	memset(&host->async_queue_head, 0, sizeof(host->async_queue_head));

	// Create an empty list by pointing the first queue head back at itself.
	host->async_queue_head.horizontal.link = (uint32_t)&host->async_queue_head;

	// And set up the defaults.
	host->async_queue_head.horizontal.type = DESCRIPTOR_QH;
	host->async_queue_head.head_reclamation_flag = 1;
	host->async_queue_head.overlay.next_dtd_pointer = TERMINATING_LINK;
	host->async_queue_head.overlay.alternate_next_dtd_pointer = TERMINATING_LINK;
	host->async_queue_head.overlay.halted = 1;

	// Pass this list to the hardware.
	USB_REG(host->controller)->ASYNCLISTADDR = host->async_queue_head.horizontal.link;
}


/**
 * Set up the linked list used to queue up periodic (interrupt) transfers.
 */
static void usb_controller_set_up_periodic_list(usb_peripheral_t *host)
{
	// Clear the structure to its all-zeroes state.
	memset(&host->periodic_queue_head, 0, sizeof(host->periodic_queue_head));

	// And set up the defaults.
	host->periodic_queue_head.horizontal.terminate = 1;
	host->periodic_queue_head.horizontal.type = DESCRIPTOR_QH;
	host->periodic_queue_head.uframe_smask = 1; // Part of the periodic schedule.
	host->periodic_queue_head.overlay.next_dtd_pointer = TERMINATING_LINK;
	host->periodic_queue_head.overlay.alternate_next_dtd_pointer = TERMINATING_LINK;
	host->periodic_queue_head.overlay.halted = 1;

	// Set up the periodic list.
	for(int i = 0; i < USB_PERIODIC_LIST_SIZE; ++i) {
		host->periodic_list[i].link = (uint32_t)&host->periodic_queue_head;
		host->periodic_list[i].type = DESCRIPTOR_QH;
		host->periodic_list[i].terminate = 0;
	}

	// Pass this list to the hardware.
	USB_REG(host->controller)->PERIODICLISTBASE = (uint32_t)&host->periodic_list;
}


/**
 * Disables the asynchronous schedule, blocking until it's fully down.
 */
void usb_host_disable_asynchronous_schedule(usb_peripheral_t *host)
{
	// Clear the asynchronous schedule enabled bit..
	USB_REG(host->controller)->USBCMD &= ~USB0_USBCMD_H_ASE;

	// ... and wait for the host to report that the schedule has been disabled.
	while(USB_REG(host->controller)->USBSTS & USB0_USBSTS_H_AS);
}


/**
 * Enables the asynchronous schedule, blocking until it fully comes up.
 */
void usb_host_enable_asynchronous_schedule(usb_peripheral_t *host)
{
	// Clear the asynchronous schedule enabled bit..
	USB_REG(host->controller)->USBCMD |= USB0_USBCMD_H_ASE;

	// ... and wait for the host to report that the schedule has been enabled.
	while(!(USB_REG(host->controller)->USBSTS & USB0_USBSTS_H_AS));
}




/**
 * Configures the USB host to support the interrupts we use for host mode.
 */
static void usb_controller_set_up_lists(usb_peripheral_t *host)
{
	// TODO: set up any resources used for isochronous transfers here, as well
	usb_controller_set_up_async_list(host);
	usb_controller_set_up_periodic_list(host);

	// Set up an emtpy linked list of pending transfers.
	host->pending_transfers.ptr = (ehci_link_t *)TERMINATING_LINK;

	// For now, trigger interrupts after a frame list of 32 elements.
	USB_REG(host->controller)->USBCMD |= (USB0_USBCMD_H_FS0 | USB1_USBCMD_H_FS2);
	USB_REG(host->controller)->USBCMD &= ~USB0_USBCMD_H_FS1;

	// Enable only the Asynchronous schedule for now. The others should be
	// set up after enumeration based on device requirements.
	USB_REG(host->controller)->USBCMD |= USB0_USBCMD_H_ASE;
}


/**
 * Initializes the USB peripheral in host mode.
 */
void usb_host_init(usb_peripheral_t *host)
{
		// Ensure the USB controller's in a state such that it can be set up
		// for host use...
		usb_phy_enable(host);
		usb_controller_reset(host);

		//... set it up for host use...
		usb_controller_set_host_mode(host);
		usb_controller_set_up_host_interrupts(host);
		usb_controller_set_up_lists(host);
}


/**
 * Issues a bus reset request, effectively asking any devices downstream
 * of the host to reset themselves.
 *
 * You should wait 100ms after calling this before interacting with the device.
 */
void usb_host_reset_device(usb_peripheral_t *host)
{
	int host_number = host->controller;

	// Disable the port, and request that it reset.
	USB_REG(host_number)->PORTSC1 &= ~USB0_PORTSC1_H_PE;
	USB_REG(host_number)->PORTSC1 |=  USB0_PORTSC1_H_PR;

	// Wait for the USB reset to complete.
	while(USB_REG(host_number)->PORTSC1 & USB0_PORTSC1_H_PR);
}


/**
 * Handle an error interrupt. Normally, we migth want to use this
 * opportunity to clear out any error'd transfer descriptors.
 */
void usb_host_handle_error(usb_peripheral_t *host)
{
		//TODO:
		(void)host;
}


/**
 * Handle interrupts for the USB host controller.
 */
static void usb_host_isr(usb_peripheral_t *host) {

	// Read (and clear) the set of active ISRs to be handled.
	const uint32_t status = usb_get_status(host);

	// TODO: Handle other interrupts?

	// If we've just finished an event on the asynchronous queue,
	// handle it.
	if (status & USB0_USBSTS_H_UAI) {
		usb_host_handle_asynchronous_transfer_complete(host);
	}

	if (status & USB0_USBSTS_H_UEI) {
		usb_host_handle_error(host);
	}


}
