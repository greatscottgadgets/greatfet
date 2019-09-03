
/*
 * This file is part of GreatFET
 *
 * Code for ULPI register interfacing via SGPIO for Rhododendron boards.
 */


#include <debug.h>
#include <errno.h>
#include <string.h>

#include <drivers/gpio.h>
#include <drivers/sgpio.h>
#include <drivers/platform_clock.h>
#include <drivers/platform_reset.h>
#include <drivers/scu.h>
#include <drivers/timer.h>
#include <drivers/arm_vectors.h>
#include <drivers/memory/allocator.h>
#include <toolchain.h>

#include "../pin_manager.h"
#include "../rhododendron.h"
#include "../usb_streaming.h"
#include "gpio_int.h"




/**
 * Rhododendron packet IDs.
 */
typedef enum {

	// Packets containing raw USB data.
	RHODODENDRON_PACKET_ID_DATA = 0,

	// Packets containing USB events.
	RHODODENDRON_PACKET_ID_RX_START   = 0x80,
	RHODODENDRON_PACKET_ID_RX_END_OK  = 0x81,
	RHODODENDRON_PACKET_ID_RX_END_ERR = 0x82,

} rhododendron_packet_id_t;


/**
 * Rhododendron pending USB events.
 */
typedef struct rhododendron_usb_event rhododendron_usb_event_t;
struct rhododendron_usb_event {

	// The position in the capture buffer associated with this event.
	// This allows us to queue events, and then add them to the USB stream
	// just before their associated event.
	uint32_t position_in_capture_buffer;

	// The position in the USB data _packet_ associated with this event.
	// This tells us which of the 32 bytes in the USB packet is associated
	// with the relevant event.
	uint32_t position_in_data_packet;

	// The core type of event this is.
	rhododendron_packet_id_t event_id;

	// The system time associated with the relevent event.
	uint32_t time;
};

/**
 * Forward delcarations.
 */

// Interrupt handlers for DIR going low or high.
static void rhododendron_direction_isr(void);


/**
 * ULPI data pins for Rhododendron boards.
 */
static sgpio_pin_configuration_t ulpi_data_pins[] = {
	{ .sgpio_pin = 0,  .scu_group = 0, .scu_pin =  0, .pull_resistors = SCU_PULLDOWN},
	{ .sgpio_pin = 1,  .scu_group = 0, .scu_pin =  1, .pull_resistors = SCU_PULLDOWN},
	{ .sgpio_pin = 2,  .scu_group = 1, .scu_pin = 15, .pull_resistors = SCU_PULLDOWN},
	{ .sgpio_pin = 3,  .scu_group = 1, .scu_pin = 16, .pull_resistors = SCU_PULLDOWN},
	{ .sgpio_pin = 4,  .scu_group = 6, .scu_pin =  3, .pull_resistors = SCU_PULLDOWN},
	{ .sgpio_pin = 5,  .scu_group = 6, .scu_pin =  6, .pull_resistors = SCU_PULLDOWN},
	{ .sgpio_pin = 6,  .scu_group = 2, .scu_pin =  2, .pull_resistors = SCU_PULLDOWN},
	{ .sgpio_pin = 7,  .scu_group = 6, .scu_pin =  8, .pull_resistors = SCU_PULLDOWN},
};


/**
 * ULPI control pins (as used here).
 */
static sgpio_pin_configuration_t ulpi_nxt_pin =
	{ .sgpio_pin = 10, .scu_group = 1, .scu_pin =  14, .pull_resistors = SCU_PULLDOWN};


static gpio_pin_t ulpi_dir_gpio      = { .port = 0, .pin = 12 };
static gpio_pin_t ulpi_nxt_alt_gpio  = { .port = 2, .pin = 15 };


/**
 * Core function to capture USB data.
 */
sgpio_function_t usb_capture_functions[] = {
	{
		.enabled                     = true,

		// Once we get to this point, we're just observing the USB data as it flies by.
		.mode                        = SGPIO_MODE_STREAM_DATA_IN,

		// We're interesting in reading data from the PHY data pins.
		.pin_configurations          = ulpi_data_pins,
		.bus_width                   = ARRAY_SIZE(ulpi_data_pins),


#ifdef RHODODENDRON_USE_USB1_CLK_AS_ULPI_CLOCK

		// We'll shift in time with rising edges of the PHY clock.
		.shift_clock_source          = SGPIO_CLOCK_SOURCE_COUNTER,
		.shift_clock_edge            = SGPIO_CLOCK_EDGE_RISING,
		.shift_clock_frequency       = 0, // Never divide; just use the SGPIO clock frequency.


#else

		// We'll shift in time with rising edges of the PHY clock.
		.shift_clock_source          = SGPIO_CLOCK_SOURCE_SGPIO08,
		.shift_clock_edge            = SGPIO_CLOCK_EDGE_RISING,
		.shift_clock_input           = &ulpi_clk_pin,

#endif

		// We're only interested in values that the PHY indicates are valid data.
		.shift_clock_qualifier       = SGPIO_QUALIFIER_SGPIO10,
		.shift_clock_qualifier_input = &ulpi_nxt_pin,
		.shift_clock_qualifier_is_active_low = false,

		// Capture our data into the USB bulk buffer, all ready to be sent up to the host.
		.buffer                      = usb_bulk_buffer,
		.buffer_order                = 15,              // 2 ^ 15 == 32768 == sizeof(usb_bulk_buffer)

		// Capture an unlimited amount of data.
		.shift_count_limit            = 0,
	},
};


/**
 * Core USB capture SGPIO configuration.
 */
sgpio_t analyzer  = {
	.functions      = usb_capture_functions,
	.function_count = ARRAY_SIZE(usb_capture_functions),
};


/**
 * Capture-state variables.
 */
volatile uint32_t usb_buffer_position;

volatile uint32_t capture_buffer_read_position = 0;
volatile uint32_t capture_buffer_write_position = 0;

volatile rhododendron_usb_event_t event_ring[128];
volatile uint32_t event_ring_read_position, event_ring_write_position, events_pending;


const uint8_t rhododendron_direction_isr_priority = 64;


// Buffer allocated for large data processing.
// Currently shared. Possibly should be replaced with malloc'd buffers?



uint32_t xxx_total_bytes_produced;


/**
 * Starts a Rhododendron capture of high-speed USB data.
 */
int rhododendron_start_capture(void)
{
	int rc;

	// Start from the beginning of our buffers.
	usb_buffer_position = 0;
	capture_buffer_read_position = 0;
	capture_buffer_write_position = 0;

	// Clear any pending events.
	event_ring_write_position = 0;
	event_ring_write_position = 0;
	events_pending = 0;

	// Set up the direction-capture interrupt.
	// TODO: abstract these interrupt priorities!
	gpio_interrupt_configure(0, ulpi_dir_gpio.port, ulpi_dir_gpio.pin,
		EDGE_SENSITIVE_RISING, rhododendron_direction_isr, rhododendron_direction_isr_priority);
	gpio_interrupt_configure(1, ulpi_dir_gpio.port, ulpi_dir_gpio.pin,
		EDGE_SENSITIVE_FALLING, rhododendron_direction_isr, rhododendron_direction_isr_priority);
	gpio_interrupt_configure(2, ulpi_nxt_alt_gpio.port, ulpi_nxt_alt_gpio.pin,
		EDGE_SENSITIVE_RISING, rhododendron_direction_isr, rhododendron_direction_isr_priority);
	gpio_interrupt_configure(3, ulpi_nxt_alt_gpio.port, ulpi_nxt_alt_gpio.pin,
		EDGE_SENSITIVE_FALLING, rhododendron_direction_isr, rhododendron_direction_isr_priority);


	// Set up the SGPIO functions used for capture...
	rc = sgpio_set_up_functions(&analyzer);
	if (rc) {
		return rc;
	}

	// Turn on our "capture triggered" LED.
	rhododendron_turn_on_led(LED_TRIGGERED);

	// Enable our DIR monitoring ISRs. Note that we don't enable ISRs 2/3 in the NVIC;
	// we want to monitor these using our existing DIR-change ISRs.
	gpio_interrupt_enable(0);
	gpio_interrupt_enable(1);

	// FIXME: verify that the Rhododendron loadable is there?

	// ... and enable USB streaming to the host.
	usb_streaming_start_streaming_to_host(
		(uint32_t *volatile)&usb_buffer_position,
		NULL);
	sgpio_run(&analyzer);

	return 0;
}


/**
 * Terminates a Rhododendron capture.
 */
void rhododendron_stop_capture(void)
{
	// Disable our stream-to-host, direction monitor, and SGPIO capture.
	sgpio_halt(&analyzer);
	gpio_interrupt_enable(0);
	gpio_interrupt_enable(1);
	usb_streaming_stop_streaming_to_host();

	// Turn off our "capture triggered" LED.
	rhododendron_turn_off_led(LED_TRIGGERED);
}


/**
 * Consumes a word from our capture buffer, and returns it.
 */

/**
 * Consumes a word from our capture buffer, and returns it.
 */
static uint32_t consume_byte(void)
{
	uint8_t byte = large_allocation_buffer[capture_buffer_read_position];
	capture_buffer_read_position = (capture_buffer_read_position + 1) % sizeof(large_allocation_buffer);

	return byte;
}


/**
 * Adds a byte to the USB upload buffer.
 */
static void produce_byte(uint8_t byte)
{
	// Add the word to the USB buffer, and move our queue ahead by one word.
	usb_bulk_buffer[usb_buffer_position] = byte;
	usb_buffer_position = (usb_buffer_position + 1) % sizeof(usb_bulk_buffer);

	++xxx_total_bytes_produced;
}


/**
 * Adds a byte to the USB upload buffer.
 */
static void produce_word(uint32_t word)
{
	uint8_t *as_bytes = (uint8_t *)&word;

	for (unsigned i = 0; i < sizeof(word); ++i) {
		produce_byte(as_bytes[i]);
	}
}





/**
 * Consumes the provided number of words from the capture buffer, and adds them
 * to our USB upload buffer.
 */
static void transfer_bytes(size_t count)
{
	while(count--) {
		produce_byte(consume_byte());
	}
}


/**
 * @return True iff the given event would take place during the next event to be processed.
 */
static bool event_in_next_data_chunk(volatile rhododendron_usb_event_t *event)
{
	// For now, we're assuming USB data chunks themselves can't
	// wrap their buffer; as they're currently designed to do so.
	// (Data is always shifted in in little chunks of 32B, so the
	//  capture that causes wrapping would implicitly put the next
	//  chunk at 0.)
	return
		(event->position_in_capture_buffer >= capture_buffer_read_position) &&
		(event->position_in_capture_buffer <  capture_buffer_write_position);
}


/**
 * Emits an event packet to the relevant host.
 */
static void emit_packet_for_event(volatile rhododendron_usb_event_t *event)
{
	//if (event->event_id == RHODODENDRON_PACKET_ID_RX_START) {
	//	produce_byte('S');
	//	produce_byte('T');
	//	produce_byte('A');
	//	produce_byte('R');
	//	produce_byte('T');
	//}
	//else {
	//	produce_byte('e');
	//	produce_byte('n');
	//	produce_byte('d');
	//}

	// Packet ID: in this case, the event ID is the packet ID.
	produce_byte(event->event_id);

	// Position in the next data packet.
	produce_byte(event->position_in_data_packet);

	// Associated time, in microseconds.
	produce_word(event->time);
}


/**
 * Emit packets to the host for any USB events that have happened recently.
 */
static void emit_pending_event_packets(void)
{

	// Try to process every event we can.
	while(events_pending) {
		volatile rhododendron_usb_event_t *next_event = &event_ring[event_ring_read_position];

		// If the next event isn't in our next data chunk, we'll have to wait for
		// the relevant capture data to appear to process our event. Since events
		// are chronologically ordered, we're done emitting events, for now.
		if (!event_in_next_data_chunk(next_event)) {
			break;
		}

		// Emit a packet for the relevant event...
		emit_packet_for_event(next_event);

		// ... and consume the relevant event.
		--events_pending;
		++event_ring_read_position;
	}
}


/**
 * Emits processed-and-packetized USB data to our host for proessing.
 */
static void emit_usb_data_packet(void)
{
	//
	// TODO: process this data more actively; allowing for e.g. filtering
	//

	// Produce our packet header...
	produce_byte(RHODODENDRON_PACKET_ID_DATA);

	// Temporary: provide a visual marker for our dump output.
	//produce_byte('U');
	//produce_byte('S');
	//produce_byte('B');
	//produce_byte('D');

	// ... and then transfer a full buffer's worth of slices.
	transfer_bytes(32);
}



/**
 * Returns the data buffer capture count.
 * Assumes the data buffer never fills or overflows.
 */
static uint32_t capture_buffer_data_count(uint32_t write_position)
{
	uint32_t virtual_write_pointer = write_position;
	uint32_t virtual_read_pointer  = capture_buffer_read_position;

	// If the capture buffer write position is _before_ the capture buffer read
	// position, then we're wrapping around the buffer's end. We'll account for
	// this by undoing the most recent modulus -- the one that caused the wrap-around.
	if (virtual_write_pointer < virtual_read_pointer) {
		virtual_write_pointer += sizeof(large_allocation_buffer);
	}

	return virtual_write_pointer - virtual_read_pointer;
}


/**
 * Core processing thread for Rhododendron. Processes USB data that has come in from
 * the M0 coprocessor; and any events that have come from either the M0 or from IRQs.
 */
void service_rhododendron(void)
{
	// Store a reference to the current write position, so we don't
	// keep reading it and blocking the M0 from accessing the bus.
	uint32_t write_position = capture_buffer_write_position;

	// Always emit any event packets relevant to us.
	emit_pending_event_packets();

	if (capture_buffer_data_count(write_position)) {
		rhododendron_toggle_led(LED_STATUS);
	}

	// While we have data to consume...
	while (capture_buffer_data_count(write_position)) {
		rhododendron_toggle_led(LED_STATUS);

		// ... handle any events that have come in, and any USB data pending.
		emit_usb_data_packet();
		emit_pending_event_packets();
	}
}


/**
 * Adds a USB event to the pending event queue.
 */
static inline void enqueue_pending_usb_event(uint8_t sgpio_position,
		rhododendron_packet_id_t packet_id)
{
	volatile rhododendron_usb_event_t *event;

	//  ... the current time, in microseconds...
	uint32_t time = get_time();

	// ... and our position in the capture buffer.
	uint32_t position_in_capture_buffer = capture_buffer_write_position;

	// Grab a write slot in our pending event ring.
	uint32_t write_position = event_ring_write_position;
	event_ring_write_position = (event_ring_write_position + 1) % ARRAY_SIZE(event_ring);

	// If we're full up on events, remove one to make room for this one.
	if (events_pending >= ARRAY_SIZE(event_ring)) {
		--events_pending;
		event_ring_read_position = (event_ring_read_position + 1) % ARRAY_SIZE(event_ring);
	}


	// Get a quick reference to it.
	event = &event_ring[write_position];

	// Finally, populate the event...
	event->event_id = packet_id;
	event->time = time;
	event->position_in_data_packet = 31 - sgpio_position;
	event->position_in_capture_buffer = position_in_capture_buffer;

	// ... and mark it as available for consumption by the other side.
	++events_pending;
}

/**
 * Core definition of the Rhododendron direction change ISR.
 * Called from a short, assembly stub that captures the SGPIO shift position prior to our prelude.
 */
void rhododendron_direction_isr_inner(uint8_t sgpio_position)
{
	const uint32_t dir_and_nxt_re_mask = (1 << 0) | (1 << 2);
	const uint32_t dir_and_nxt_fe_mask = (1 << 1) | (1 << 3);

	uint32_t active_interrupts = GPIO_PIN_INTERRUPT_IST;

	if (dir_and_nxt_re_mask & active_interrupts) {
		enqueue_pending_usb_event(sgpio_position, RHODODENDRON_PACKET_ID_RX_START);
	}

	if (dir_and_nxt_fe_mask & active_interrupts) {
		enqueue_pending_usb_event(sgpio_position, RHODODENDRON_PACKET_ID_RX_END_OK);
	}

	// Mark all of our interrupts as serviced.
	GPIO_PIN_INTERRUPT_IST = active_interrupts;
}


void ATTR_NAKED rhododendron_direction_isr(void)
{
	uint32_t sgpio_position;

	// Grab the current SGPIO shift position, and prepare to pass it to our C code.
	asm volatile (
		"ldr %0, =0x401011c0\n\t"
		"ldr %0, [%0]\n\t"
		: "+r" (sgpio_position)
		:
		:
	);
	rhododendron_direction_isr_inner(sgpio_position);
}
