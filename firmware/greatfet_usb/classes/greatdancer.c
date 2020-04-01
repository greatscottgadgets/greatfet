/*
 * This file is part of GreatFET
 */


#include <drivers/comms.h>
#include <errno.h>
#include <stddef.h>
#include <debug.h>
#include <string.h>

#include <drivers/usb/usb_queue.h>
#include <libopencm3/cm3/vector.h>
#include <libopencm3/lpc43xx/m4/nvic.h>

#include <greatfet_core.h>

#include <drivers/usb/usb.h>
#include <drivers/usb/usb_standard_request.h>
#include "../usb_descriptor.h"
#include "../usb_device.h"
#include "../usb_endpoint.h"
#include <drivers/usb/usb_request.h>

#include <glitchkit.h>

#define CLASS_NUMBER_SELF (0x104)

typedef char packet_buffer[4096];

/**
 *	Buffers for each of the relevant endpoints.
 *	TODO: can these be allocated on-demand to decrease our total RAM reservation?
 *	or should they be moved into the AHB ram?
 */
packet_buffer endpoint_buffer[NUM_USB1_ENDPOINTS];
uint32_t total_received_data[NUM_USB1_ENDPOINTS];


/**
 * Enumeration describing each of the possible Index values for GET_STATUS
 * requests.
 */
enum greatdancer_status_request {
	GET_USBSTS = 0,
	GET_ENDPTSETUPSTAT = 1,
	GET_ENDPTCOMPLETE = 2,
	GET_ENDPTSTATUS = 3,
	GET_ENDPTNAK = 4
};

enum greatdancer_quirks {
	MANUAL_SET_ADDRESS = 0x01,
};


/* Stores the current status of the USB controller, as managed by our interrupts. */
static volatile uint32_t endptnak_deferred;
static volatile uint32_t usbsts_deferred;

/* True iff we should automatically and transparently handle SET_ADDRESS requests. */
static bool automatically_handle_set_address = true;

/**
 * When using the GreatDancer, all events are generated
 * and handled on the host side, so we don't need to generate
 * anything.
 *
 * TODO: this should be removable; we should now be able to use the weak default
 */
const usb_request_handlers_t usb1_request_handlers = {
	.standard = NULL,
	.class = NULL,
	.vendor = NULL,
	.reserved = NULL,
};

static void greatdancer_usb_isr(void);

/**
 * Perform all of the one-time initializations of the GreatDancer API.
 */
static void init_greatdancer_api(void) {
	// Initialize all of our queues, so they're ready
	// if the GreatDancer application decides to use them.
	usb_queue_init(&usb1_endpoint_control_out_queue);
	usb_queue_init(&usb1_endpoint_control_in_queue);
	usb_queue_init(&usb1_endpoint1_out_queue);
	usb_queue_init(&usb1_endpoint1_in_queue);
	usb_queue_init(&usb1_endpoint2_out_queue);
	usb_queue_init(&usb1_endpoint2_in_queue);
	usb_queue_init(&usb1_endpoint3_out_queue);
	usb_queue_init(&usb1_endpoint3_in_queue);
}
CALL_ON_INIT(init_greatdancer_api);

/**
 * Finds the endpoint object associated with a given address.
 * This version can be used even before the USB controller is initialized,
 * but is less flexible.
 *
 * @param address The address for which an endpoint should be located.
 */
static usb_endpoint_t *usb_preinit_endpoint_from_address(uint8_t address)
{
	switch(address) {
		case 0x80: return &usb1_endpoint_control_in;
		case 0x00: return &usb1_endpoint_control_out;

		case 0x81: return &usb1_endpoint1_in;
		case 0x01: return &usb1_endpoint1_out;

		case 0x82: return &usb1_endpoint2_in;
		case 0x02: return &usb1_endpoint2_out;

		case 0x83: return &usb1_endpoint3_in;
		case 0x03: return &usb1_endpoint3_out;
	}

	return NULL;
}



/**
 * Performs the per-run initialization of the GreatDancer device.
 * Should be between successive executions of the facedancer.
 *
 * @param max_packet_size The maximum packet size to be used on endpoint zero.
 */
static void set_up_greatdancer_device(uint16_t max_packet_size) {
  usb_endpoint_t *ep0_in, *ep0_out;

	usb_peripheral_reset(&usb_peripherals[1]);
	usb_device_init(&usb_peripherals[1]);

	// XXX temporary
	glitchkit_enable();
	glitchkit_enable_trigger_on(GLITCHKIT_USBDEVICE_FINISH_TD);

	// Set up the control endpoint. The application will request setup
	// for all of the non-standard channels on connection.
	// Fetch the endpoint to be configured, and configure it.
	ep0_out	= usb_preinit_endpoint_from_address((uint8_t)0x00);
	ep0_in  = usb_preinit_endpoint_from_address((uint8_t)0x80);
	if(!ep0_in || !ep0_out) {
	  return;
	}

	// And initialize the endpoint.
	usb_endpoint_init_without_descriptor(ep0_in,  max_packet_size, USB_TRANSFER_TYPE_CONTROL, false);

	// We'll handle zero-length-packets manually for the out control endpoint.
	usb_endpoint_init_without_descriptor(ep0_out, max_packet_size, USB_TRANSFER_TYPE_CONTROL, true);
}


static int greatdancer_verb_connect(struct command_transaction *trans)
{
	uint16_t ep0_max_packet_size, quirk_flags;
	ep0_max_packet_size = comms_argument_parse_uint16_t(trans);
	quirk_flags			= comms_argument_parse_uint16_t(trans);

	if (!comms_transaction_okay(trans)) {
		return EBADMSG;
	}

	usb_controller_reset(&usb_peripherals[1]);
	set_up_greatdancer_device(ep0_max_packet_size);

	// Apply the platform quirks we'll be using.
	automatically_handle_set_address = !(quirk_flags & MANUAL_SET_ADDRESS);

	// Set up our IRQ handler and enable the USB controller.
	// From this point forward, the greatdancer_usb_isr can be generated.
	usb_set_irq_handler(&usb_peripherals[1], greatdancer_usb_isr);
	usb_run(&usb_peripherals[1]);

	return 0;
}


/**
 * Sets up the GreatDancer to make a USB connection.
 *
 * Expects zero or more triplets describing how the device's endpoints should
 * be initialied. Each triplet should contain:
 *
 * - One byte of endpoint address
 * - Two bytes describing the maximum packet size on the endpoint
 * - One byte describing the endpoint type
 */
static int greatdancer_verb_set_up_endpoints(struct command_transaction *trans)
{
	// XXX abstract me away
	USB1_ENDPTNAKEN = 0x00;
	USB1_ENDPTNAK  |= 0xFFFFFFFF;

	// Continue for as long as we have endpoint triplets to handle.
	while (comms_argument_data_remaining(trans)) {

		uint8_t address, transfer_type;
		uint16_t max_packet_size;
		usb_endpoint_t *target_endpoint;


		// Parse the endpoint descriptor triplet...
		address = comms_argument_parse_uint8_t(trans);
		max_packet_size = comms_argument_parse_uint16_t(trans);
		transfer_type = comms_argument_parse_uint8_t(trans);

		if (!comms_transaction_okay(trans)) {
			return EBADMSG;
		}

		// Endpoint zero is always the control endpoint, and can't be configured.
		// Ignore any attempt to configure the control endpoint.
		if((address & 0x7F) == 0x00) {
			pr_warning("greatdancer: ignoring request to reconfigure control endpoint!\n");
			continue;
		}

		// Ignore any endpoint configurations that try and set up transactions
		// we won't be able to handle.
		if(max_packet_size > sizeof(packet_buffer)) {
			pr_error("greatdancer: failing to set up endpoint with packet size %d; larger than max (%d)!\n",
					max_packet_size, sizeof(packet_buffer));
			return EINVAL;
		}

		// Fetch the endpoint to be configured, and configure it.
		target_endpoint = usb_preinit_endpoint_from_address(address);
		if(!target_endpoint) {
			pr_error("greatdancer: failed to configure impossible endpoint with address %d\n", address);
			return EINVAL;
		}

		// And initialize the endpoint.
		usb_endpoint_init_without_descriptor(target_endpoint, max_packet_size, transfer_type, false);

		// Finally, enable notifications when NAKs occur for this endpoint.
		// TODO: Make this optional?
		if(address & 0x80) {
			usb_in_endpoint_enable_nak_interrupt(target_endpoint);
		}
	}

	return 0;
}


/**
 * Terminates all existing communication and shuts down the GreatDancer USB.
 */
static int greatdancer_verb_disconnect(struct command_transaction *trans)
{
	(void)trans;
	usb_controller_reset(&usb_peripherals[1]);
	return 0;
}


/**
 * Handle requests for the USB controller's status. Normally, we'd
 * read this from the USBSTS register, and clear the USBSTS register;
 * but our interrupt routine should have already done this for us.
 *
 * Instead, read its results.
 */
static uint32_t greatdancer_get_usb_status()
{
	// Read the currently pending events, and clear them.
	return __sync_fetch_and_and(&usbsts_deferred, ~USB1_USBINTR_D);
}


/**
 * Handle requests for the USB controller's status. Normally, we'd
 * read this from the USBSTS register, and clear the USBSTS register;
 * but our interrupt routine should have already done this for us.
 *
 * Instead, read its results.
 */
static uint32_t greatdancer_get_nak_status()
{
	// Read the currently pending events, and clear them.
	return __sync_fetch_and_and(&endptnak_deferred, ~USB1_ENDPTNAKEN);
}


/**
 * Retrieves the value of a the stauts register corresponding to a given
 * greatfet_status_request_t.
 *
 * @param index Selects the status register to be queried.
 * @param device The device for which the register is to be queried.
 * @return The value of the status register.
 */
static uint32_t get_status_register(enum greatdancer_status_request index,
		const usb_peripheral_t* const device)
{
	switch(index) {
		case GET_USBSTS: return greatdancer_get_usb_status();
		case GET_ENDPTSETUPSTAT: return usb_get_endpoint_setup_status(device);
		case GET_ENDPTCOMPLETE: return usb_get_endpoint_complete(device);
		case GET_ENDPTSTATUS: return usb_get_endpoint_ready(device);
		case GET_ENDPTNAK: return greatdancer_get_nak_status();
	}

	// TODO: more meaningfull error handling, here?
	return -1;
}


/**
 * Queries the GreatDancer for any events that need to be processed.
 * FIXME: should this actually use an interrupt pipe?
 *
 * The index value is used to select which status section we're looking for:
 *
 *	0 = pending interrupts (USBSTS register)
 *	1 = setup status for all endpoints (ENDPTSETUPSTAT)
 *	2 = endpoint completion status (ENDPTCOMPLETE)
 *	3 = endpoint primed status (ENDPTSTATUS)
 *
 *	Always transmits a 4-byte word back to the host.
 */
static int greatdancer_verb_get_status(struct command_transaction *trans)
{
	uint8_t register_type = comms_argument_parse_uint8_t(trans);
	uint32_t status;

	if (!comms_transaction_okay(trans)) {
		return EBADMSG;
	}

	// Read from the status register, and return the read value.
	status = get_status_register(register_type, &usb_peripherals[1]);
	comms_response_add_uint32_t(trans, status);

	return 0;
}


/**
 * Reads a setup packet from the GreatDancer port and relays it to the host.
 * The index parameter specifies which endpoint we should be reading from.
 *
 * Always transmits an 8-byte setup packet back to the host. If no setup packet
 * is waiting, the results of this vendor request are unspecified.
 */
static int greatdancer_verb_read_setup(struct command_transaction *trans)
{
	uint_fast8_t address;

	usb_setup_t *setup_data;
	usb_endpoint_t *target_endpoint;

	int endpoint_number = comms_argument_parse_uint8_t(trans);

	if (!comms_transaction_okay(trans)) {
		return EBADMSG;
	}

	// Figure out the endpoint we're reading setup data from...
	address = usb_endpoint_address(USB_TRANSFER_DIRECTION_OUT, endpoint_number);
	target_endpoint = usb_endpoint_from_address(address, &usb_peripherals[1]);

	if (!target_endpoint) {
		pr_error("greatdancer: trying to read a setup packet from an impossible endpoint %x!\n", address);
	}

	// ... and find its setup data.
	setup_data = (usb_setup_t *)usb_queue_head(target_endpoint->address, target_endpoint->device)->setup;

	// Validate that we got a sane pointer from the USB stack.
	if (!setup_data) {
		pr_error("greatdancer: internal error -- USB API passed us a NULL setup packet pointer!\n");
		return EFAULT;
	}

	// Reserve space for the target data...
	comms_response_add_raw(trans, setup_data, sizeof(*setup_data));

	// ... and mark that packet as handled.
	usb_clear_endpoint_setup_status(1 << endpoint_number, &usb_peripherals[1]);

	return 0;
}


/**
 * Callback that's executed each time a nonblocking read completes.
 * Stores the number of bytes transferred when the read executed.
 */
static void store_transfer_count_callback(void * const user_data, unsigned int transferred)
{
		unsigned int * total_data = (unsigned int*)user_data;

		if(total_data) {
			*total_data = transferred;
		}
}


/**
 * Primes the USB controller to recieve data on a particular endpoint, but
 * does not wait for a transfer to complete. The transfer's status can be
 * checked with get_transfer_status and then read with finish_nonblocking_read.
 *
 * The index parameter specifies which endpoint we should be reading from.
 */
static int greatdancer_verb_start_nonblocking_read(struct command_transaction *trans)
{
		uint_fast8_t address;
		uint32_t maximum_length;

		usb_endpoint_t *target_endpoint;

		int endpoint_number = comms_argument_parse_uint8_t(trans);

		if (!comms_transaction_okay(trans)) {
			return EBADMSG;
		}

		// Figure out the endpoint we're reading setup data from...
		address = usb_endpoint_address(USB_TRANSFER_DIRECTION_OUT, endpoint_number);
		target_endpoint = usb_endpoint_from_address(address, &usb_peripherals[1]);

		maximum_length = (endpoint_number == 0) ? (target_endpoint->max_packet_size) :
			sizeof(endpoint_buffer[endpoint_number]);

		// ... and start a nonblocking transfer.
		usb_transfer_schedule(target_endpoint, &endpoint_buffer[endpoint_number], maximum_length,
				store_transfer_count_callback, &total_received_data[endpoint_number]);
		return 0;
}


/**
 * Queries an endpoint to determine how much data is available. Should only be
 * used after a nonblocking read was primed with START_NONBLOCKING_READ and
 * completed by the USB hardware. Completion can be checked by reading the
 * ENDPTCOMPLETE register with GET_STATUS.
 *
 * Response is invalid unless a transfer has been initiated with
 * START_NONBLOCKING_READ and then has completed.
 */
static int greatdancer_verb_get_nonblocking_data_length(struct command_transaction *trans)
{
	int endpoint_number = comms_argument_parse_uint8_t(trans);
	if (!comms_transaction_okay(trans)) {
		return EBADMSG;
	}

	// Transmit the read data back.
	comms_response_add_uint32_t(trans, total_received_data[endpoint_number]);
	return 0;
}


/**
 * Finishes a non-blocking read by returning the read data back to the host.
 * Should only be used after determining that a transfer is complete with
 * the get_transfer_status request and reading the relevant length with
 * get_nonblocking_data_length.
 *
 * index: The endpoint number to request data on.
 * length: The total amount of bytes to read, which must be <= the value returned
 *		by a get_nonblocking_data_lengthr request.
 */
static int greatdancer_verb_finish_nonblocking_read(struct command_transaction *trans)
{
	int endpoint_number = comms_argument_parse_uint8_t(trans);
	if (!comms_transaction_okay(trans)) {
		return EBADMSG;
	}

	// Transmit the read data back.
	comms_response_add_raw(trans, &endpoint_buffer[endpoint_number], total_received_data[endpoint_number]);
	return 0;
}


/**
 * Reads data from the GreatFET host and sends on a provided GreatDancer endpoint.
 * The index parameter specifies which endpoint we should be reading from.
 *
 * index: The endpoint to be transmitted.
 * The OUT request should contain a data stage containing all data to be sent.
 */
static int greatdancer_verb_send_on_endpoint(struct command_transaction *trans)
{
	uint_fast8_t address;
	uint32_t length_to_send;
	usb_endpoint_t *target_endpoint;

	int endpoint_number = comms_argument_parse_uint8_t(trans);
	void *data_to_send = comms_argument_read_buffer(trans, -1, &length_to_send);

	if (!comms_transaction_okay(trans) || !data_to_send) {
		return EBADMSG;
	}

	// If we've been asked to send more than we can fit in the relevant buffer,
	// fail out.
	if (length_to_send > sizeof(packet_buffer)) {
		pr_warning("gpio: host requested to send %d, but our maximum packet size is %d!\n", length_to_send,
				sizeof(packet_buffer));
		return ENOSPC;
	}

	// Figure out the endpoint we're reading setup data from...
	address = usb_endpoint_address(USB_TRANSFER_DIRECTION_IN, endpoint_number);
	target_endpoint = usb_endpoint_from_address(address, &usb_peripherals[1]);

	// Copy our setup data into our DMA'able buffer.
	memcpy(&endpoint_buffer[endpoint_number], data_to_send, length_to_send);

	// And request that the USB controller send it.
	usb_transfer_schedule(target_endpoint, &endpoint_buffer[endpoint_number], length_to_send, NULL, NULL);
	return 0;
}


static int greatdancer_verb_set_address(struct command_transaction *trans)
{
	uint_fast8_t address = comms_argument_parse_uint16_t(trans);
	uint8_t deferred	 = comms_argument_parse_uint8_t(trans);

	if (!comms_transaction_okay(trans)) {
		return EBADMSG;
	}

	// If the host requested a deferred reset, handle this after
	// the given SETUP transaction is compelte. Otherwise, have the
	// SET_ADDRESS take effect immediately.
	if(deferred)
		usb_set_address_deferred(&usb_peripherals[1], address);
	else
		usb_set_address_immediate(&usb_peripherals[1], address);

	return 0;
}


static int greatdancer_verb_bus_reset(struct command_transaction *trans)
{
	(void)trans;

	usb_bus_reset(&usb_peripherals[1]);
	return 0;
}


/**
 * Temporarily stalls the relevant USB endpoint.
 */
static int greatdancer_verb_stall_endpoint(struct command_transaction *trans)
{
	uint_fast8_t address;
	usb_endpoint_t *target_endpoint;

	int endpoint_number = comms_argument_parse_uint8_t(trans);

	if (!comms_transaction_okay(trans)) {
		return EBADMSG;
	}

	// Figure out the endpoint we're about to stall...
	address = usb_endpoint_address(USB_TRANSFER_DIRECTION_OUT, endpoint_number);
	target_endpoint = usb_endpoint_from_address(address, &usb_peripherals[1]);

	// ... and stall it.
	usb_endpoint_stall(target_endpoint);
	return 0;
}


/**
 * Should be called whenever a transfer is complete; cleans up any transfer
 * descriptors associated with that transfer.
 */
static int greatdancer_verb_clean_up_transfer(struct command_transaction *trans)
{
		uint_fast8_t endpoint_address = comms_argument_parse_uint8_t(trans);
		int endpoint_number = endpoint_address & 0x7F;

		if (!comms_transaction_okay(trans)) {
			return EBADMSG;
		}

		// Figure out the endpoint we're reading setup data from...
		usb_endpoint_t* const target_endpoint = usb_endpoint_from_address(endpoint_address, &usb_peripherals[1]);

		// Clear the "transfer complete" bit.
		if(endpoint_address & 0x80) {
			usb_clear_endpoint_complete(USB1_ENDPTCOMPLETE_ETCE(1 << endpoint_number), &usb_peripherals[1]);
		} else {
			usb_clear_endpoint_complete(USB1_ENDPTCOMPLETE_ERCE(1 << endpoint_number), &usb_peripherals[1]);
		}

		// Clean up any transfers that are complete on the given endpoint.
		usb_queue_transfer_complete(target_endpoint);
		return 0;
}

/**
 * There are a few events we'll often want to handle asynchronously of
 * the host-- including events with very tight timing requirements. We
 * check for them here, and handle them if appropriate.
 */
static void greatdancer_check_for_asynchronous_events()
{

	// Read the status of all current endpoints...
	const uint32_t endptsetupstat = usb_get_endpoint_setup_status(&usb_peripherals[1]);
	const uint32_t endptsetupstat_bit = USB1_ENDPTSETUPSTAT_ENDPTSETUPSTAT(1 << 0);

	// Quirk 1:
	// Check for SET_ADDRESS events on EP0.

	// If we have an event on EP0::out, check to see if it's a setup event.
	if( endptsetupstat && endptsetupstat_bit ) {
		usb_endpoint_t* const endpoint = &usb1_endpoint_control_out;

		usb_copy_setup(&endpoint->setup, usb_queue_head(endpoint->address, endpoint->device)->setup);
		usb_copy_setup(&endpoint->in->setup, usb_queue_head(endpoint->address, endpoint->device)->setup);

		// Check to see if this is a standard SET_ADDRESS request,
		// and check to see if we want to handle set_address requests.
		if( (endpoint->setup.request == USB_STANDARD_REQUEST_SET_ADDRESS) &&
				(endpoint->setup.request_type == 0x00) &&
				automatically_handle_set_address) {
			uint8_t address = endpoint->setup.value_l;

			// Mark this particular event as handled...
			usb_clear_endpoint_setup_status(endptsetupstat_bit, &usb_peripherals[1]);

			// Transparently adopt the address specified for this device.
			usb_set_address_deferred(&usb_peripherals[1], address);
			usb_transfer_schedule_ack(endpoint->in);
		}
	}
}

static void greatdancer_handle_naks()
{
		uint32_t status = USB1_ENDPTNAK;// & USB1_ENDPTNAKEN;

		// Store the array of NAKs that have happened...
		__sync_fetch_and_or(&endptnak_deferred, status);

		//... and mark the relevant interrupts as serviced.
		USB1_ENDPTNAK = status;
}

/**
 * Handle interrupts for the Greatdancer's USB controller.
 */
static void greatdancer_usb_isr(void) {
	const uint32_t status = usb_get_status(&usb_peripherals[1]);

	if( status == 0 ) {
		// Nothing to do.
		return;
	}

	// If a USB event has happened, handle it.
	if( status & USB1_USBSTS_D_UI ) {
		glitchkit_notify_event(GLITCHKIT_USBDEVICE_FINISH_TD);
		greatdancer_check_for_asynchronous_events();
	}

	// If we've issued a NAK, service the relevant interrupt.
	if (status & USB1_USBSTS_D_NAKI ) {
		greatdancer_handle_naks();
	}

	// We've now handled everything we wanted to-- and cleared the USBSTS register
	// of its status bits. This is necessary, so this interrupt doesn't re-fire
	// continuously, but it also steals information that the host wants. We thus
	// store the remaining USBSTS bits our our module's state.
	__sync_fetch_and_or(&usbsts_deferred, status);
}



/**
 * Verbs for the GPIO API.
 */
static struct comms_verb _verbs[] = {

		/* Connection/disconnection. */
		{  .name = "connect", .handler = greatdancer_verb_connect, .in_signature = "<HH",
		   .out_signature = "", .in_param_names = "ep0_max_packet_size, quirk_flags",
		   .doc = "Sets up the target port to connect to a host.\nEnables the target port's USB pull-ups." },
		{  .name = "disconnect", .handler = greatdancer_verb_disconnect, .in_signature = "",
		   .out_signature = "", .doc = "Disconnects the target port from the host." },
		{  .name = "bus_reset", .handler = greatdancer_verb_bus_reset, .in_signature = "",
		   .out_signature = "", .doc = "Causes the target device to handle a bus reset.\n"
		   "Usually issued when the host requests a bus reset." },

		/* Enumeration / setup. */
		{  .name = "set_address", .handler = greatdancer_verb_set_address, .in_signature = "<BB",
		   .out_signature = "", .in_param_names = "address, deferred",
		   .doc = "Sets the address of the target device.\n"
		   "If deferred is set, this action won't complete until setup phase ends." },
		{  .name = "set_up_endpoints", .handler = greatdancer_verb_set_up_endpoints, .in_signature = "<*(BHB)",
		   .out_signature = "", .in_param_names = "endpoint_descriptors",
		   .doc = "Sets up all of the non-control endpoints for the device.\n"
		   "Accepts endpoint triplets of (address, max_packet_size, transfer_type)." },

		/* Status & control. */
		{  .name = "get_status", .handler = greatdancer_verb_get_status, .in_signature = "<B",
		   .out_signature = "<I", .in_param_names = "register_type", .out_param_names = "register_value",
		   .doc = "Reads one of the device's USB status registers." },
		{  .name = "read_setup", .handler = greatdancer_verb_read_setup, .in_signature = "<B",
		   .out_signature = "<8X", .in_param_names = "endpoint_number", .out_param_names = "raw_setup_packet",
		   .doc = "Reads any pending setup packets recieved on the given endpoint." },
		{  .name = "stall_endpoint", .handler = greatdancer_verb_stall_endpoint, .in_signature = "<B",
		   .out_signature = "", .in_param_names = "endpoint_address",
		   .doc = "Stalls the endpoint with the provided address." },

		/* Data transfers. */
		{  .name = "send_on_endpoint", .handler = greatdancer_verb_send_on_endpoint, .in_signature = "<B*X",
		   .out_signature = "", .in_param_names = "endpoint_number, data_to_send",
		   .doc = "Sends the provided data on the given IN endpoint." },
		{  .name = "clean_up_transfer", .handler = greatdancer_verb_clean_up_transfer, .in_signature = "<B",
		   .out_signature = "", .in_param_names = "endpoint_address",
		   .doc = "Cleans up any complete transfers on the given endpoint." },
		{  .name = "start_nonblocking_read", .handler = greatdancer_verb_start_nonblocking_read, .in_signature = "<B",
		   .out_signature = "", .in_param_names = "endpoint_number",
		   .doc = "Begins listening for data on the given OUT endpoint.\n" },
		{  .name = "finish_nonblocking_read", .handler = greatdancer_verb_finish_nonblocking_read, .in_signature = "<B",
		   .out_signature = "<*X", .in_param_names = "endpoint_number", .out_param_names = "read_data",
		   .doc = "Returns the data read after a given non-blocking read.\n" },
		{  .name = "get_nonblocking_data_length", .handler = greatdancer_verb_get_nonblocking_data_length, .in_signature = "<B",
		   .out_signature = "<I", .in_param_names = "endpoint_number", .out_param_names = "length",
		   .doc = "Returns the amount of data read after a given non-blocking read.\n" },

		/* Sentinel. */
		{}
};
COMMS_DEFINE_SIMPLE_CLASS(greatdancer, CLASS_NUMBER_SELF, "greatdancer", _verbs,
		"API for fine-grained control of the Target USB port.");

