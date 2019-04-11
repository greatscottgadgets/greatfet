/*
 * This file is part of GreatFET
 */

#include <stddef.h>
#include <greatfet_core.h>
#include <i2c_bus.h>
#include <libopencm3/lpc43xx/uart.h>
#include <errno.h>
#include <drivers/comms.h>


#define CLASS_NUMBER_SELF (0x110)

static uint16_t duty_cycle_count;
static uint8_t read_status;
static uint8_t write_status;

// static int i2c_verb_start(struct command_transaction *trans)
// {
// 	uint16_t value = comms_argument_parse_uint16_t(trans);
// 	if (value == 0) {
// 		duty_cycle_count = 255;
// 	} else {
// 		duty_cycle_count = value;
// 	}
// 	i2c_bus_start(&i2c0, duty_cycle_count);

// 	return 0;
// }

static int uart_verb_init(struct command_transation *trans)
{
    uint8_t uart_num 			= comms_argument_parse_uint8_t(trans);
    uint8_t number_of_data_bits	= comms_argument_parse_uint8_t(trans);
    uint8_t number_of_stop_bits = comms_argument_parse_uint8_t(trans);
    uint8_t parity_bit			= comms_argument_parse_uint8_t(trans);
    uint8_t divisor				= comms_argument_parse_uint8_t(trans);
    uint8_t divaddval			= comms_argument_parse_uint8_t(trans);
    uint8_t mulval				= comms_argument_parse_uint8_t(trans);

	uart_init(uart_num, number_of_data_bits, number_of_stop_bits, parity_bit, divisor, divaddval, mulval);
    return 0;
}

// static int i2c_verb_stop()
// {
// 	i2c_bus_stop(&i2c0);
// 	return 0;	
// }

// static int i2c_verb_read(struct command_transaction *trans)
// {
// 	uint16_t address 		= comms_argument_parse_uint16_t(trans);
// 	uint16_t rx_length 		= comms_argument_parse_uint16_t(trans);
// 	uint8_t *i2c_rx_buffer 	= comms_response_reserve_space(trans, rx_length);
// 	// TODO: data validation

// 	if (!comms_transaction_okay(trans)) {
//         return EBADMSG;
//     }
// 	read_status = i2c_bus_read(&i2c0, address, i2c_rx_buffer, rx_length);
// 	comms_response_add_uint8_t(trans, read_status);

// 	return 0;
// }

// static int i2c_verb_write(struct command_transaction *trans)
// {
// 	uint32_t tx_length;
// 	uint16_t address 		= comms_argument_parse_uint16_t(trans);
// 	uint8_t *data_to_write 	= comms_argument_read_buffer(trans, -1, &tx_length);
// 	// TODO: data validation

// 	if (!comms_transaction_okay(trans)) {
//         return EBADMSG;
//     }
// 	write_status = i2c_bus_write(&i2c0, address, data_to_write, tx_length);
// 	comms_response_add_uint8_t(trans, write_status);

// 	return 0;
// }

// static int i2c_verb_scan(struct command_transaction *trans)
// {
// 	uint8_t *write_status_buffer = comms_response_reserve_space(trans, 16);
// 	uint8_t *read_status_buffer = comms_response_reserve_space(trans, 16);
// 	uint8_t address;

// 	if (!comms_transaction_okay(trans)) {
//         return EBADMSG;
//     }

// 	for (address = 0; address < 16; address++) {
// 		write_status_buffer[address] = 0;
// 		read_status_buffer[address] = 0;
// 	}

// 	for (address = 0; address < 128; address++) {
// 		write_status = i2c_bus_write(&i2c0, address, NULL, 0);
// 		if (write_status == 0x18) {
// 			write_status_buffer[address >> 3] |= 1 << (address & 0x07);
// 		}

// 		read_status = i2c_bus_read(&i2c0, address, NULL, 0);
// 		if (read_status == 0x40) {
// 			read_status_buffer[address >> 3] |= 1 << (address & 0x07);
// 		}
// 	}

// 	return 0;
// }

/**
 * Verbs for the firmware API.
 */
static struct comms_verb _verbs[] = {
		{ .name = "start", .handler = uart_verb_init,
			.in_signature = "<I", .out_signature = "",
			.in_param_names = "value, duty_cycle_count",
			.doc = "Initialize and transmit a start bit to an I2C device" },
		{} // Sentinel
};
COMMS_DEFINE_SIMPLE_CLASS(uart, CLASS_NUMBER_SELF, "uart", _verbs,
		"API for UART communication.");

