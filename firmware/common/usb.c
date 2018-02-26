/*
 * This file is part of GreatFET
 */

#include <stdint.h>
#include <stdbool.h>

#include "usb.h"
#include "usb_host.h"
#include "usb_type.h"
#include "usb_queue.h"
#include "usb_registers.h"
#include "usb_standard_request.h"
#include "greatfet_core.h"

#include <libopencm3/lpc43xx/creg.h>
#include <libopencm3/lpc43xx/m4/nvic.h>
#include <libopencm3/lpc43xx/rgu.h>
#include <libopencm3/lpc43xx/usb.h>
#include <libopencm3/lpc43xx/scu.h>

// FIXME: Clean me up to use the USB_REG macro from usb_registers.h to reduce duplication!

#define USB_QH_INDEX(endpoint_address) (((endpoint_address & 0xF) * 2) + ((endpoint_address >> 7) & 1))

usb_queue_head_t* usb_queue_head(
	const uint_fast8_t endpoint_address,
	const usb_peripheral_t* const device
) {
	usb_queue_head_t * endpoint_list = device->queue_heads_device;
	return &endpoint_list[USB_QH_INDEX(endpoint_address)];
}

usb_endpoint_t* usb_endpoint_from_address(
	const uint_fast8_t endpoint_address,
	const usb_peripheral_t* const device
) {
	return (usb_endpoint_t*)usb_queue_head(endpoint_address, device)->_reserved_0;
}

uint_fast8_t usb_endpoint_address(
	const usb_transfer_direction_t direction,
	const uint_fast8_t number
) {
	return ((direction == USB_TRANSFER_DIRECTION_IN) ? 0x80 : 0x00) + number;
}

static bool usb_endpoint_is_in(const uint_fast8_t endpoint_address) {
	return (endpoint_address & 0x80) ? true : false;
}

static uint_fast8_t usb_endpoint_number(const uint_fast8_t endpoint_address) {
	return (endpoint_address & 0xF);
}

void usb_peripheral_reset(const usb_peripheral_t* const device) {
	if( device->controller == 0 ) {
		RESET_CTRL0 = RESET_CTRL0_USB0_RST;
		RESET_CTRL0 = 0;
		while( (RESET_ACTIVE_STATUS0 & RESET_CTRL0_USB0_RST) == 0 );
	}
	if( device->controller == 1 ) {
		RESET_CTRL0 = RESET_CTRL0_USB1_RST;
		RESET_CTRL0 = 0;
		while( (RESET_ACTIVE_STATUS0 & RESET_CTRL0_USB1_RST) == 0 );
	}
}

void usb_phy_enable(const usb_peripheral_t* const device) {
	if(device->controller == 0) {
		CREG_CREG0 &= ~CREG_CREG0_USB0PHY;
	}
	if(device->controller == 1) {
		/* Enable the USB1 FS PHY. */
		SCU_SFSUSB = 0x12;

#ifdef BOARD_CAPABILITY_USB1_SENSE_VBUS
		/*
		 * HACK: The USB1 PHY will only run if we tell it VBUS is
		 * present by setting SFSUSB bit 5. Shortly, we should use
		 * the USB1_SENSE pin to drive an interrupt that adjusts this
		 * bit to match the sense pin's value. For now, we'll lie and
		 * say VBUS is always there.
		 */
		SCU_SFSUSB |= (1 << 5);
#else
		/*
		 * If we don't have the ability to sense VBUS, lie and pretend that we
		 * always detect it. This actually works pretty perfectly for pretty much
		 * all USB hosts, even if it's in violation of the spec-- which says we
		 * shouldn't drive current through D+/D- until VBUS is present.
		 */
		SCU_SFSUSB |= (1 << 5);
#endif
	}
}

static void usb_clear_pending_interrupts(const uint32_t mask,
										 const usb_peripheral_t* const device) {
	if(device->controller == 0) {
		USB0_ENDPTNAK = mask;
		USB0_ENDPTNAKEN = mask;
		USB0_USBSTS_D = mask;
		USB0_ENDPTSETUPSTAT = USB0_ENDPTSETUPSTAT & mask;
		USB0_ENDPTCOMPLETE = USB0_ENDPTCOMPLETE & mask;
	}
	if(device->controller == 1) {
		USB1_ENDPTNAK = mask;
		USB1_ENDPTNAKEN = mask;
		USB1_USBSTS_D = mask;
		USB1_ENDPTSETUPSTAT = USB1_ENDPTSETUPSTAT & mask;
		USB1_ENDPTCOMPLETE = USB1_ENDPTCOMPLETE & mask;
	}
}

static void usb_clear_all_pending_interrupts(const usb_peripheral_t* const device) {
	usb_clear_pending_interrupts(0xFFFFFFFF, device);
}

static void usb_wait_for_endpoint_priming_to_finish(const uint32_t mask,
													const usb_peripheral_t* const device) {
	// Wait until controller has parsed new transfer descriptors and prepared
	// receive buffers.
	if(device->controller == 0) {
		while( USB0_ENDPTPRIME & mask );		
	}
	if(device->controller == 1) {
		while( USB1_ENDPTPRIME & mask );
	}
}

static void usb_flush_endpoints(const uint32_t mask,
								const usb_peripheral_t* const device) {
	// Clear any primed buffers. If a packet is in progress, that transfer
	// will continue until completion.
	if(device->controller == 0) {
		USB0_ENDPTFLUSH = mask;		
	}
	if(device->controller == 1) {
		USB1_ENDPTFLUSH = mask;
	}
}

static void usb_wait_for_endpoint_flushing_to_finish(const uint32_t mask,
													 const usb_peripheral_t* const device) {
	// Wait until controller has flushed all endpoints / cleared any primed
	// buffers.
	if(device->controller == 0) {
		while( USB0_ENDPTFLUSH & mask );		
	}
	if(device->controller == 1) {
		while( USB1_ENDPTFLUSH & mask );
	}
}

static void usb_flush_primed_endpoints(const uint32_t mask,
									   const usb_peripheral_t* const device) {
	usb_wait_for_endpoint_priming_to_finish(mask, device);
	usb_flush_endpoints(mask, device);
	usb_wait_for_endpoint_flushing_to_finish(mask, device);
}

static void usb_flush_all_primed_endpoints(const usb_peripheral_t* const device) {
	usb_flush_primed_endpoints(0xFFFFFFFF, device);
}

static void usb_endpoint_set_type(
	const usb_endpoint_t* const endpoint,
	const usb_transfer_type_t transfer_type
) {
	// NOTE: UM10503 section 23.6.24 "Endpoint 1 to 5 control registers" says
	// that the disabled side of an endpoint must be set to a non-control type
	// (e.g. bulk, interrupt, or iso).
	const uint_fast8_t endpoint_number = usb_endpoint_number(endpoint->address);
	if(endpoint->device->controller == 0) {
		USB0_ENDPTCTRL(endpoint_number)
			= ( USB0_ENDPTCTRL(endpoint_number)
			  & ~(USB0_ENDPTCTRL_TXT1_0_MASK | USB0_ENDPTCTRL_RXT_MASK)
			  )
			| ( USB0_ENDPTCTRL_TXT1_0(transfer_type)
			  | USB0_ENDPTCTRL_RXT(transfer_type)
			  );
	}
	if(endpoint->device->controller == 1) {
		USB1_ENDPTCTRL(endpoint_number)
			= ( USB1_ENDPTCTRL(endpoint_number)
			  & ~(USB1_ENDPTCTRL_TXT1_0_MASK | USB1_ENDPTCTRL_RXT_MASK)
			  )
			| ( USB1_ENDPTCTRL_TXT1_0(transfer_type)
			  | USB1_ENDPTCTRL_RXT(transfer_type)
			  );
	}
}

static void usb_endpoint_enable(
	const usb_endpoint_t* const endpoint
) {
	const uint_fast8_t endpoint_number = usb_endpoint_number(endpoint->address);
	if( endpoint->device->controller == 0 ) {
		if( usb_endpoint_is_in(endpoint->address) ) {
			USB0_ENDPTCTRL(endpoint_number) |= (USB0_ENDPTCTRL_TXE | USB0_ENDPTCTRL_TXR);
		} else {
			USB0_ENDPTCTRL(endpoint_number) |= (USB0_ENDPTCTRL_RXE | USB0_ENDPTCTRL_RXR);
		}
	}
	if( endpoint->device->controller == 1 ) {
		if( usb_endpoint_is_in(endpoint->address) ) {
			USB1_ENDPTCTRL(endpoint_number) |= (USB1_ENDPTCTRL_TXE | USB1_ENDPTCTRL_TXR);
		} else {
			USB1_ENDPTCTRL(endpoint_number) |= (USB1_ENDPTCTRL_RXE | USB1_ENDPTCTRL_RXR);
		}
	}
}

static void usb_endpoint_clear_pending_interrupts(
	const usb_endpoint_t* const endpoint
) {
	const uint_fast8_t endpoint_number = usb_endpoint_number(endpoint->address);
	if(endpoint->device->controller == 0) {
		if( usb_endpoint_is_in(endpoint->address) ) {
			usb_clear_pending_interrupts(USB0_ENDPTCOMPLETE_ETCE(1 << endpoint_number),
										 endpoint->device);
		} else {
			usb_clear_pending_interrupts(USB0_ENDPTCOMPLETE_ERCE(1 << endpoint_number),
										 endpoint->device);
		}
	}
	if(endpoint->device->controller == 1) {
		if( usb_endpoint_is_in(endpoint->address) ) {
			usb_clear_pending_interrupts(USB1_ENDPTCOMPLETE_ETCE(1 << endpoint_number),
										 endpoint->device);
		} else {
			usb_clear_pending_interrupts(USB1_ENDPTCOMPLETE_ERCE(1 << endpoint_number),
										 endpoint->device);
		}
	}
		
}

void usb_endpoint_disable(
	const usb_endpoint_t* const endpoint
) {
	const uint_fast8_t endpoint_number = usb_endpoint_number(endpoint->address);
	if(endpoint->device->controller == 0) {
		if( usb_endpoint_is_in(endpoint->address) ) {
			USB0_ENDPTCTRL(endpoint_number) &= ~(USB0_ENDPTCTRL_TXE);
		} else {
			USB0_ENDPTCTRL(endpoint_number) &= ~(USB0_ENDPTCTRL_RXE);
		}
	}
	if(endpoint->device->controller == 1) {
		if( usb_endpoint_is_in(endpoint->address) ) {
			USB1_ENDPTCTRL(endpoint_number) &= ~(USB1_ENDPTCTRL_TXE);
		} else {
			USB1_ENDPTCTRL(endpoint_number) &= ~(USB1_ENDPTCTRL_RXE);
		}
	}
        usb_queue_flush_endpoint(endpoint);
	usb_endpoint_clear_pending_interrupts(endpoint);
	usb_endpoint_flush(endpoint);
}

void usb_endpoint_prime(
	const usb_endpoint_t* const endpoint,
	usb_transfer_descriptor_t* const first_td	
) {
	usb_queue_head_t* const qh = usb_queue_head(endpoint->address, endpoint->device);
	
	qh->next_dtd_pointer = first_td;
	qh->total_bytes
		&= ~( USB_TD_DTD_TOKEN_STATUS_ACTIVE
		    | USB_TD_DTD_TOKEN_STATUS_HALTED
			)
		;
	
	const uint_fast8_t endpoint_number = usb_endpoint_number(endpoint->address);
	if(endpoint->device->controller == 0) {
		if( usb_endpoint_is_in(endpoint->address) ) {
			USB0_ENDPTPRIME = USB0_ENDPTPRIME_PETB(1 << endpoint_number);
		} else {
			USB0_ENDPTPRIME = USB0_ENDPTPRIME_PERB(1 << endpoint_number);
		}
	}
	if(endpoint->device->controller == 1) {
		if( usb_endpoint_is_in(endpoint->address) ) {
			USB1_ENDPTPRIME = USB1_ENDPTPRIME_PETB(1 << endpoint_number);
		} else {
			USB1_ENDPTPRIME = USB1_ENDPTPRIME_PERB(1 << endpoint_number);
		}
	}
}

static bool usb_endpoint_is_priming(
	const usb_endpoint_t* const endpoint
) {
	const uint_fast8_t endpoint_number = usb_endpoint_number(endpoint->address);
	if(endpoint->device->controller == 0) {
		if( usb_endpoint_is_in(endpoint->address) ) {
			return USB0_ENDPTPRIME & USB0_ENDPTPRIME_PETB(1 << endpoint_number);
		} else {
			return USB0_ENDPTPRIME & USB0_ENDPTPRIME_PERB(1 << endpoint_number);
		}
	}
	else {
		if( usb_endpoint_is_in(endpoint->address) ) {
			return USB1_ENDPTPRIME & USB1_ENDPTPRIME_PETB(1 << endpoint_number);
		} else {
			return USB1_ENDPTPRIME & USB1_ENDPTPRIME_PERB(1 << endpoint_number);
		}
	}
}

// Schedule an already filled-in transfer descriptor for execution on
// the given endpoint, waiting until the endpoint has finished.
void usb_endpoint_schedule_wait(
	const usb_endpoint_t* const endpoint,
	usb_transfer_descriptor_t* const td
) {
	// Ensure that endpoint is ready to be primed.
	// It may have been flushed due to an aborted transaction.
	// TODO: This should be preceded by a flush?
	while( usb_endpoint_is_ready(endpoint) );

	td->next_dtd_pointer = USB_TD_NEXT_DTD_POINTER_TERMINATE;

	usb_endpoint_prime(endpoint, td);
}

// Schedule an already filled-in transfer descriptor for execution on
// the given endpoint, appending to the end of the endpoint's queue if
// there are pending TDs. Note that this requires that one knows the
// tail of the endpoint's TD queue. Moreover, the user is responsible
// for setting the TERMINATE bit of next_dtd_pointer if needed.
void usb_endpoint_schedule_append(
	const usb_endpoint_t* const endpoint,
	usb_transfer_descriptor_t* const tail_td,
	usb_transfer_descriptor_t* const new_td
) {
	bool done = 0;

	tail_td->next_dtd_pointer = new_td;

	if (usb_endpoint_is_priming(endpoint)) {
		return;
	}

	if(endpoint->device->controller == 0) {
		do {
			USB0_USBCMD_D |= USB0_USBCMD_D_ATDTW;
			done = usb_endpoint_is_ready(endpoint);
		} while (!(USB0_USBCMD_D & USB0_USBCMD_D_ATDTW));
	
		USB0_USBCMD_D &= ~USB0_USBCMD_D_ATDTW;
	}
	if(endpoint->device->controller == 1) {
		do {
			USB1_USBCMD_D |= USB1_USBCMD_D_ATDTW;
			done = usb_endpoint_is_ready(endpoint);
		} while (!(USB1_USBCMD_D & USB1_USBCMD_D_ATDTW));
	
		USB1_USBCMD_D &= ~USB1_USBCMD_D_ATDTW;
	}
	if(!done) {
		usb_endpoint_prime(endpoint, new_td);
	}
}

void usb_endpoint_flush(
	const usb_endpoint_t* const endpoint
) {
	const uint_fast8_t endpoint_number = usb_endpoint_number(endpoint->address);
	usb_queue_flush_endpoint(endpoint);
	if(endpoint->device->controller == 0) {
		if( usb_endpoint_is_in(endpoint->address) ) {
			usb_flush_primed_endpoints(USB0_ENDPTFLUSH_FETB(1 << endpoint_number),
									   endpoint->device);
		} else {
			usb_flush_primed_endpoints(USB0_ENDPTFLUSH_FERB(1 << endpoint_number),
									   endpoint->device);
		}
	}
	if(endpoint->device->controller == 1) {
		if( usb_endpoint_is_in(endpoint->address) ) {
			usb_flush_primed_endpoints(USB1_ENDPTFLUSH_FETB(1 << endpoint_number),
									   endpoint->device);
		} else {
			usb_flush_primed_endpoints(USB1_ENDPTFLUSH_FERB(1 << endpoint_number),
									   endpoint->device);
		}
	}
}
/*
static bool usb_endpoint_is_flushing(
	const usb_endpoint_t* const endpoint
) {
	const uint_fast8_t endpoint_number = usb_endpoint_number(endpoint->address);
	if( usb_endpoint_is_in(endpoint->address) ) {
		return USB0_ENDPTFLUSH & USB0_ENDPTFLUSH_FETB(1 << endpoint_number);
	} else {
		return USB0_ENDPTFLUSH & USB0_ENDPTFLUSH_FERB(1 << endpoint_number);
	}
}
*/
bool usb_endpoint_is_ready(
	const usb_endpoint_t* const endpoint
) {
	const uint_fast8_t endpoint_number = usb_endpoint_number(endpoint->address);
	if(endpoint->device->controller == 0) {
		if( usb_endpoint_is_in(endpoint->address) ) {
			return USB0_ENDPTSTAT & USB0_ENDPTSTAT_ETBR(1 << endpoint_number);
		} else {
			return USB0_ENDPTSTAT & USB0_ENDPTSTAT_ERBR(1 << endpoint_number);
		}
	} else {
		if( usb_endpoint_is_in(endpoint->address) ) {
			return USB1_ENDPTSTAT & USB1_ENDPTSTAT_ETBR(1 << endpoint_number);
		} else {
			return USB1_ENDPTSTAT & USB1_ENDPTSTAT_ERBR(1 << endpoint_number);
		}
	}
}

bool usb_endpoint_is_complete(
	const usb_endpoint_t* const endpoint
) {
	const uint_fast8_t endpoint_number = usb_endpoint_number(endpoint->address);
	if(endpoint->device->controller == 0) {
		if( usb_endpoint_is_in(endpoint->address) ) {
			return USB0_ENDPTCOMPLETE & USB0_ENDPTCOMPLETE_ETCE(1 << endpoint_number);
		} else {
			return USB0_ENDPTCOMPLETE & USB0_ENDPTCOMPLETE_ERCE(1 << endpoint_number);
		}
	} else {
		if( usb_endpoint_is_in(endpoint->address) ) {
			return USB1_ENDPTCOMPLETE & USB1_ENDPTCOMPLETE_ETCE(1 << endpoint_number);
		} else {
			return USB1_ENDPTCOMPLETE & USB1_ENDPTCOMPLETE_ERCE(1 << endpoint_number);
		}
	}
}

void usb_endpoint_stall(
	const usb_endpoint_t* const endpoint
) {
	// Endpoint is to be stalled as a pair -- both OUT and IN.
	// See UM10503 section 23.10.5.2 "Stalling"
	const uint_fast8_t endpoint_number = usb_endpoint_number(endpoint->address);
	if(endpoint->device->controller == 0) {
		USB0_ENDPTCTRL(endpoint_number) |= (USB0_ENDPTCTRL_RXS | USB0_ENDPTCTRL_TXS);
	}
	if(endpoint->device->controller == 1) {
		USB1_ENDPTCTRL(endpoint_number) |= (USB1_ENDPTCTRL_RXS | USB1_ENDPTCTRL_TXS);
	}
	
	// TODO: Also need to reset data toggle in both directions?
}

void usb_controller_run(const usb_peripheral_t* const device) {
	USB_REG(device->controller)->USBCMD |= USB0_USBCMD_D_RS;
}

static void usb_controller_stop(const usb_peripheral_t* const device) {
	if( device->controller == 0) {
		USB0_USBCMD_D &= ~USB0_USBCMD_D_RS;
	}
	if( device->controller == 1) {
		USB1_USBCMD_D &= ~USB1_USBCMD_D_RS;
	}
}

static uint_fast8_t usb_controller_is_resetting(const usb_peripheral_t* const device) {
	if( device->controller == 0) {
		return (USB0_USBCMD_D & USB0_USBCMD_D_RST) != 0;
	} else {
		return (USB1_USBCMD_D & USB1_USBCMD_D_RST) != 0;
	}
}

static void usb_controller_set_device_mode(usb_peripheral_t* device) {

	// Mark the peripheral as in DEVICE mode.
	device->mode = USB_CONTROLLER_MODE_DEVICE;

	// And disable any host-mode pull-downs used.
	usb_host_disable_pulldowns(device);

	if( device->controller == 0) {
		// Set USB0 peripheral mode
		USB0_USBMODE_D = USB0_USBMODE_D_CM1_0(2);
		
		// Set device-related OTG flags
		// OTG termination: controls pull-down on USB_DM
		USB0_OTGSC = USB0_OTGSC_OT;
	}
	if( device->controller == 1) {
		// Set USB1 peripheral mode
		USB1_USBMODE_D = USB1_USBMODE_D_CM1_0(2);
	}
}

usb_speed_t usb_speed(
	const usb_peripheral_t* const device
) {
	if( device->controller == 0 ) {
		switch( USB0_PORTSC1_D & USB0_PORTSC1_D_PSPD_MASK ) {
		case USB0_PORTSC1_D_PSPD(0):
			return USB_SPEED_FULL;
		
		case USB0_PORTSC1_D_PSPD(2):
			return USB_SPEED_HIGH;
		
		default:
			// TODO: What to do/return here? Is this even possible?
			return USB_SPEED_FULL;
		}
	} else {
		// TODO: This should not be possible with a more class-like
		// implementation.
		return USB_SPEED_FULL;
	}
}

uint32_t usb_get_status(const usb_peripheral_t* const device) {
	uint32_t status = 0;
	int usb_number = device->controller;

	// Read the status of the activated interrupts...
	status = USB_REG(usb_number)->USBSTS & USB_REG(usb_number)->USBINTR;

	// Clear flags that were just read, leaving alone any flags that
	// were just set (after the read). It's important to read and
	// reset flags atomically! :-)
	USB_REG(usb_number)->USBSTS = status;
	return status;
}

void usb_clear_endpoint_setup_status(const uint32_t endpoint_setup_status,
											const usb_peripheral_t* const device) {
 	if( device->controller == 0 ) {
		USB0_ENDPTSETUPSTAT = endpoint_setup_status;
	}
	if( device->controller == 1 ) {
		USB1_ENDPTSETUPSTAT = endpoint_setup_status;
	}
}

uint32_t usb_get_endpoint_setup_status(const usb_peripheral_t* const device) {
 	if( device->controller == 0 ) {
		return USB0_ENDPTSETUPSTAT;
	} else {
		return USB1_ENDPTSETUPSTAT;
	}
}

void usb_clear_endpoint_complete(const uint32_t endpoint_complete,
										const usb_peripheral_t* const device) {
	if( device->controller == 0 ) {
		USB0_ENDPTCOMPLETE = endpoint_complete;
	}
	if( device->controller == 1 ) {
		USB1_ENDPTCOMPLETE = endpoint_complete;
	}
}

uint32_t usb_get_endpoint_complete(const usb_peripheral_t* const device) {
	if( device->controller == 0 ) {
		return USB0_ENDPTCOMPLETE;
	} else {
		return USB1_ENDPTCOMPLETE;
	}
}


uint32_t usb_get_endpoint_ready(const usb_peripheral_t* const device) {
	if( device->controller == 0 ) {
		return USB0_ENDPTSTAT;
	} else {
		return USB1_ENDPTSTAT;
	}
}

static void usb_disable_all_endpoints(const usb_peripheral_t* const device) {
	// Endpoint 0 is always enabled. TODO: So why set ENDPTCTRL0?
	if( device->controller == 0 ) {
		USB0_ENDPTCTRL0 &= ~(USB0_ENDPTCTRL0_RXE | USB0_ENDPTCTRL0_TXE);
		USB0_ENDPTCTRL1 &= ~(USB0_ENDPTCTRL1_RXE | USB0_ENDPTCTRL1_TXE);
		USB0_ENDPTCTRL2 &= ~(USB0_ENDPTCTRL2_RXE | USB0_ENDPTCTRL2_TXE);
		USB0_ENDPTCTRL3 &= ~(USB0_ENDPTCTRL3_RXE | USB0_ENDPTCTRL3_TXE);
		USB0_ENDPTCTRL4 &= ~(USB0_ENDPTCTRL4_RXE | USB0_ENDPTCTRL4_TXE);
		USB0_ENDPTCTRL5 &= ~(USB0_ENDPTCTRL5_RXE | USB0_ENDPTCTRL5_TXE);
	}
	if( device->controller == 1 ) {
		USB1_ENDPTCTRL0 &= ~(USB1_ENDPTCTRL0_RXE | USB1_ENDPTCTRL0_TXE);
		USB1_ENDPTCTRL1 &= ~(USB1_ENDPTCTRL1_RXE | USB1_ENDPTCTRL1_TXE);
		USB1_ENDPTCTRL2 &= ~(USB1_ENDPTCTRL2_RXE | USB1_ENDPTCTRL2_TXE);
		USB1_ENDPTCTRL3 &= ~(USB1_ENDPTCTRL3_RXE | USB1_ENDPTCTRL3_TXE);
	}
}

void usb_set_address_immediate(
	const usb_peripheral_t* const device,
	const uint_fast8_t address
) {
	if( device->controller == 0 ) {
		USB0_DEVICEADDR = USB0_DEVICEADDR_USBADR(address);
	}
	if( device->controller == 1 ) {
		USB1_DEVICEADDR = USB1_DEVICEADDR_USBADR(address);
	}
}

void usb_set_address_deferred(
	const usb_peripheral_t* const device,
	const uint_fast8_t address
) {
	if( device->controller == 0 ) {
		USB0_DEVICEADDR
			= USB0_DEVICEADDR_USBADR(address)
		    | USB0_DEVICEADDR_USBADRA
			;
	}
	if( device->controller == 1 ) {
		USB1_DEVICEADDR
			= USB1_DEVICEADDR_USBADR(address)
		    | USB1_DEVICEADDR_USBADRA
			;
	}
}

static void usb_reset_all_endpoints(
	const usb_peripheral_t* const device
) {
	usb_disable_all_endpoints(device);
	usb_clear_all_pending_interrupts(device);
	usb_flush_all_primed_endpoints(device);
}

void usb_controller_reset(
	usb_peripheral_t* const device
) {
	// TODO: Good to disable some USB interrupts to avoid priming new
	// new endpoints before the controller is reset?
	usb_reset_all_endpoints(device);
	usb_controller_stop(device);

	// Reset controller. Resets internal pipelines, timers, counters, state
	// machines to initial values. Not recommended when device is in attached
	// state -- effect on attached host is undefined. Detach first by flushing
	// all primed endpoints and stopping controller.
	if( device->controller == 0 ) {
		USB0_USBCMD_D = USB0_USBCMD_D_RST;
	}
	if( device->controller == 1 ) {
		USB1_USBCMD_D = USB1_USBCMD_D_RST;
	}

	while( usb_controller_is_resetting(device) );
}

void usb_bus_reset(
	usb_peripheral_t* const device
) {
	// According to UM10503 v1.4 section 23.10.3 "Bus reset":
	usb_reset_all_endpoints(device);
	usb_set_address_immediate(device, 0);
	usb_set_configuration(device, 0);
	
	// TODO: Enable endpoint 0, which might not actually be necessary,
	// as the datasheet claims it can't be disabled.

	//wait_ms(3);
	//
	//if( USB0_PORTSC1 & USB0_PORTSC1_PR ) {
	//	// Port still is in the reset state.
	//} else {
	//	usb_hardware_reset();
	//}
}

void usb_set_irq_handler(
	usb_peripheral_t* const device,
	vector_table_entry_t isr
) {
	if( device->controller == 0 ) {
		vector_table.irq[NVIC_USB0_IRQ] = isr;
	}
	if( device->controller == 1 ) {
		vector_table.irq[NVIC_USB1_IRQ] = isr;
	}
}

static void usb_interrupt_enable(
	usb_peripheral_t* const device
) {
	if( device->controller == 0 ) {
		nvic_enable_irq(NVIC_USB0_IRQ);
	}
	if( device->controller == 1 ) {
		nvic_enable_irq(NVIC_USB1_IRQ);
	}
}

void usb_device_init(
	usb_peripheral_t* const device
) {
	if( device->controller == 0 ) {
		//usb_peripherals[0] = device;
	
		usb_phy_enable(device);
		usb_controller_reset(device);
		usb_controller_set_device_mode(device);

	
		// Set interrupt threshold interval to 0
		USB0_USBCMD_D &= ~USB0_USBCMD_D_ITC_MASK;

		// Configure endpoint list address 
		USB0_ENDPOINTLISTADDR = (uint32_t)&device->queue_heads_device;
	
		// Enable interrupts
		USB0_USBINTR_D =
			  USB0_USBINTR_D_UE
			| USB0_USBINTR_D_UEE
			| USB0_USBINTR_D_PCE
			| USB0_USBINTR_D_URE
			//| USB0_USBINTR_D_SRE
			| USB0_USBINTR_D_SLE
			//| USB0_USBINTR_D_NAKE
			;
	}
	if( device->controller == 1 ) {
		//usb_peripherals[1] = device;
	
		usb_phy_enable(device);
		usb_controller_reset(device);
		usb_controller_set_device_mode(device);
	
		// Set interrupt threshold interval to 0
		USB1_USBCMD_D &= ~USB1_USBCMD_D_ITC_MASK;

		// Configure endpoint list address 
		USB1_ENDPOINTLISTADDR = (uint32_t)&device->queue_heads_device;
	
		// Enable interrupts
		// TODO: Check to see if these actually need to generate interrupts
		// for us to read their status out of USBSTS.
		USB1_USBINTR_D =
			  USB1_USBINTR_D_UE
			| USB1_USBINTR_D_UEE
			| USB1_USBINTR_D_PCE
			| USB1_USBINTR_D_URE
			//| USB1_USBINTR_D_SRE
			| USB1_USBINTR_D_SLE
			| USB1_USBINTR_D_NAKE
			;
	}
}

void usb_run(
	usb_peripheral_t* const device
) {
	usb_interrupt_enable(device);
	usb_controller_run(device);
}

void usb_copy_setup(usb_setup_t* const dst, const volatile uint8_t* const src) {
	dst->request_type = src[0];
	dst->request = src[1];
	dst->value_l = src[2];
	dst->value_h = src[3];
	dst->index_l = src[4];
	dst->index_h = src[5];
	dst->length_l = src[6];
	dst->length_h = src[7];
}


void usb_endpoint_init_without_descriptor(
	const usb_endpoint_t* const endpoint,
  uint_fast16_t max_packet_size,
  usb_transfer_type_t transfer_type
) {
	usb_endpoint_flush(endpoint);
	
	// TODO: There are more capabilities to adjust based on the endpoint
	// descriptor.
	usb_queue_head_t* const qh = usb_queue_head(endpoint->address, endpoint->device);
	qh->capabilities
		= USB_QH_CAPABILITIES_MULT(0)
		| USB_QH_CAPABILITIES_ZLT
		| USB_QH_CAPABILITIES_MPL(max_packet_size)
		| ((transfer_type == USB_TRANSFER_TYPE_CONTROL) ? USB_QH_CAPABILITIES_IOS : 0)
		;
	qh->current_dtd_pointer = 0;
	qh->next_dtd_pointer = USB_TD_NEXT_DTD_POINTER_TERMINATE;
	qh->total_bytes
		= USB_TD_DTD_TOKEN_TOTAL_BYTES(0)
		| USB_TD_DTD_TOKEN_MULTO(0)
		;
	qh->buffer_pointer_page[0] = 0;
	qh->buffer_pointer_page[1] = 0;
	qh->buffer_pointer_page[2] = 0;
	qh->buffer_pointer_page[3] = 0;
	qh->buffer_pointer_page[4] = 0;
	
	// This is how we look up an endpoint structure from an endpoint address:
	qh->_reserved_0 = (uint32_t)endpoint;

	usb_endpoint_set_type(endpoint, transfer_type);
	
	usb_endpoint_enable(endpoint);
}


void usb_in_endpoint_enable_nak_interrupt(
	const usb_endpoint_t* const endpoint
) {
	uint8_t endpoint_number = usb_endpoint_number(endpoint->address);

	if( endpoint->device->controller == 0 ) {
		USB0_ENDPTNAKEN |= USB0_ENDPTNAKEN_EPTNE(1 << endpoint_number);
	} else {
		USB1_ENDPTNAKEN |= USB1_ENDPTNAKEN_EPTNE(1 << endpoint_number);
	}
}

void usb_in_endpoint_disable_nak_interrupt(
	const usb_endpoint_t* const endpoint
) {
	uint8_t endpoint_number = usb_endpoint_number(endpoint->address);

	if( endpoint->device->controller == 0 ) {
		USB0_ENDPTNAKEN &= ~USB0_ENDPTNAKEN_EPTNE(1 << endpoint_number);
	} else {
		USB1_ENDPTNAKEN &= ~USB1_ENDPTNAKEN_EPTNE(1 << endpoint_number);
	}
}

void usb_endpoint_init(
	const usb_endpoint_t* const endpoint
) {
	usb_endpoint_flush(endpoint);

	uint_fast16_t max_packet_size = endpoint->device->descriptor[7];
	usb_transfer_type_t transfer_type = USB_TRANSFER_TYPE_CONTROL;
	const uint8_t* const endpoint_descriptor = usb_endpoint_descriptor(endpoint);
	if( endpoint_descriptor ) {
		max_packet_size = usb_endpoint_descriptor_max_packet_size(endpoint_descriptor);
		transfer_type = usb_endpoint_descriptor_transfer_type(endpoint_descriptor);
	}

  usb_endpoint_init_without_descriptor(endpoint, max_packet_size, transfer_type);

}

static void usb_check_for_setup_events(const usb_peripheral_t* const device) {
	const uint32_t endptsetupstat = usb_get_endpoint_setup_status(device);
	uint32_t endptsetupstat_bit = 0;
	if( endptsetupstat ) {
		for( uint_fast8_t i=0; i<6; i++ ) {
			if(device->controller == 0) {
				endptsetupstat_bit = USB0_ENDPTSETUPSTAT_ENDPTSETUPSTAT(1 << i);
			}
			if(device->controller == 1) {
				endptsetupstat_bit = USB1_ENDPTSETUPSTAT_ENDPTSETUPSTAT(1 << i);
			}
			if( endptsetupstat & endptsetupstat_bit ) {
				usb_endpoint_t* const endpoint = 
					usb_endpoint_from_address(
						usb_endpoint_address(USB_TRANSFER_DIRECTION_OUT, i),
						device);
				if( endpoint && endpoint->setup_complete ) {
					usb_copy_setup(&endpoint->setup,
							   usb_queue_head(endpoint->address, endpoint->device)->setup);
					// TODO: Clean up this duplicated effort by providing
					// a cleaner way to get the SETUP data.
					usb_copy_setup(&endpoint->in->setup,
							   usb_queue_head(endpoint->address, endpoint->device)->setup);
					usb_clear_endpoint_setup_status(endptsetupstat_bit, device);
					endpoint->setup_complete(endpoint);
				} else {
					usb_clear_endpoint_setup_status(endptsetupstat_bit, device);
				}
			}
		}
	}
}

static void usb_check_for_transfer_events(const usb_peripheral_t* const device) {
	const uint32_t endptcomplete = usb_get_endpoint_complete(device);
	uint32_t endptcomplete_out_bit = 0;
	uint32_t endptcomplete_in_bit = 0;
	if( endptcomplete ) {
		for( uint_fast8_t i=0; i<6; i++ ) {
			if(device->controller == 0) {
				endptcomplete_out_bit = USB0_ENDPTCOMPLETE_ERCE(1 << i);
			}
			if(device->controller == 1) {
				endptcomplete_out_bit = USB1_ENDPTCOMPLETE_ERCE(1 << i);
			}
			if( endptcomplete & endptcomplete_out_bit ) {
				usb_clear_endpoint_complete(endptcomplete_out_bit, device);
			 	usb_endpoint_t* const endpoint = 
					usb_endpoint_from_address(
						usb_endpoint_address(USB_TRANSFER_DIRECTION_OUT, i),
						device);
				if( endpoint && endpoint->transfer_complete ) {
					endpoint->transfer_complete(endpoint);
				}
			}

			if(device->controller == 0) {
				endptcomplete_in_bit = USB0_ENDPTCOMPLETE_ETCE(1 << i);
			}
			if(device->controller == 1) {
				endptcomplete_in_bit = USB1_ENDPTCOMPLETE_ETCE(1 << i);
			}
			if( endptcomplete & endptcomplete_in_bit ) {
				usb_clear_endpoint_complete(endptcomplete_in_bit, device);
				usb_endpoint_t* const endpoint = 
					usb_endpoint_from_address(
						usb_endpoint_address(USB_TRANSFER_DIRECTION_IN, i),
						device);
				if( endpoint && endpoint->transfer_complete ) {
					endpoint->transfer_complete(endpoint);
				}
			}
		}
	}
}

void usb0_isr() {
	const uint32_t status = usb_get_status(&usb_peripherals[0]);
	
	if( status == 0 ) {
		// Nothing to do.
		return;
	}
	
	if( status & USB0_USBSTS_D_UI ) {
		// USB:
		// - Completed transaction transfer descriptor has IOC set.
		// - Short packet detected.
		// - SETUP packet received.

		usb_check_for_setup_events(&usb_peripherals[0]);
		usb_check_for_transfer_events(&usb_peripherals[0]);
		
		// TODO: Reset ignored ENDPTSETUPSTAT and ENDPTCOMPLETE flags?
	}

	if( status & USB0_USBSTS_D_SRI ) {
		// Start Of Frame received.
	}

	if( status & USB0_USBSTS_D_PCI ) {
		// Port change detect:
		// Port controller entered full- or high-speed operational state.
	}

	if( status & USB0_USBSTS_D_SLI ) {
		// Device controller suspend.
	}

	if( status & USB0_USBSTS_D_URI ) {
		// USB reset received.
		usb_bus_reset(&usb_peripherals[0]);
	}

	if( status & USB0_USBSTS_D_UEI ) {
		// USB error:
		// Completion of a USB transaction resulted in an error condition.
		// Set along with USBINT if the TD on which the error interrupt
		// occurred also had its interrupt on complete (IOC) bit set.
		// The device controller detects resume signalling only.
	}

	if( status & USB0_USBSTS_D_NAKI ) {
		// Both the TX/RX endpoint NAK bit and corresponding TX/RX endpoint
		// NAK enable bit are set.
	}
}

void usb1_isr() {
  return;
	const uint32_t status = usb_get_status(&usb_peripherals[1]);
	
	if( status == 0 ) {
		// Nothing to do.
		return;
	}
	
	if( status & USB1_USBSTS_D_UI ) {
		// USB:
		// - Completed transaction transfer descriptor has IOC set.
		// - Short packet detected.
		// - SETUP packet received.

		usb_check_for_setup_events(&usb_peripherals[1]);
		usb_check_for_transfer_events(&usb_peripherals[1]);
		
		// TODO: Reset ignored ENDPTSETUPSTAT and ENDPTCOMPLETE flags?
	}

	if( status & USB1_USBSTS_D_SRI ) {
		// Start Of Frame received.
	}

	if( status & USB1_USBSTS_D_PCI ) {
		// Port change detect:
		// Port controller entered full- or high-speed operational state.
	}

	if( status & USB1_USBSTS_D_SLI ) {
		// Device controller suspend.
	}

	if( status & USB1_USBSTS_D_URI ) {
		// USB reset received.
		usb_bus_reset(&usb_peripherals[1]);
	}

	if( status & USB1_USBSTS_D_UEI ) {
		// USB error:
		// Completion of a USB transaction resulted in an error condition.
		// Set along with USBINT if the TD on which the error interrupt
		// occurred also had its interrupt on complete (IOC) bit set.
		// The device controller detects resume signalling only.
	}

	if( status & USB1_USBSTS_D_NAKI ) {
		// Both the TX/RX endpoint NAK bit and corresponding TX/RX endpoint
		// NAK enable bit are set.
	}
}
