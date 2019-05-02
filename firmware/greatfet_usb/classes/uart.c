/*
 * This file is part of GreatFET
 */

#include <stddef.h>
#include <greatfet_core.h>
#include <i2c_bus.h>
#include <libopencm3/lpc43xx/uart.h>
#include <libopencm3/lpc43xx/scu.h>
#include <errno.h>
#include <drivers/comms.h>
#include <pins.h>


#define CLASS_NUMBER_SELF (0x110)

static uint16_t duty_cycle_count;
static uint8_t read_status;
static uint8_t write_status;

static int uart_verb_init(struct command_transaction *trans)
{
    uint32_t uart_num 			= comms_argument_parse_uint32_t(trans);
    uint8_t number_of_data_bits	= comms_argument_parse_uint8_t(trans);
    uint8_t number_of_stop_bits = comms_argument_parse_uint8_t(trans);
    uint8_t parity_bit			= comms_argument_parse_uint8_t(trans);
    uint16_t baud			    = comms_argument_parse_uint16_t(trans);

    switch(uart_num) {
        case 0:
            uart_num = UART0_NUM;
            break;
        case 1:
            uart_num = UART1_NUM;
            break;
        case 2:
            uart_num = UART2_NUM;
            break;
        case 3:
            uart_num = UART3_NUM;
            break;
    }
    
    switch(number_of_data_bits) {
        case 5:
            number_of_data_bits = UART_DATABIT_5;
            break;
        case 6:
            number_of_data_bits = UART_DATABIT_6;
            break;
        case 7:
            number_of_data_bits = UART_DATABIT_7;
            break;
        case 8:
            number_of_data_bits = UART_DATABIT_8;
            break;
    }

    switch(number_of_stop_bits) {
        case 1:
            number_of_stop_bits = UART_STOPBIT_1;
            break;
        case 2:
            number_of_stop_bits = UART_STOPBIT_2;
            break;
    }

    switch(parity_bit) {
        case 0:
            parity_bit = UART_PARITY_NONE;
            break;
        case 1:
            parity_bit = UART_PARITY_ODD;
            break;
        case 2:
            parity_bit = UART_PARITY_EVEN;
            break;
        case 3:
            parity_bit = UART_PARITY_SP_1;
            break;
        case 4:
            parity_bit = UART_PARITY_SP_0;
            break;
    }

    // TODO: allow fine tuning of divisor with divaddval and mulval
    uint16_t divisor = (204000000/(16*baud));
    uint8_t divaddval = 0;
    uint8_t mulval = 0;

    uart_init(uart_num, number_of_data_bits, number_of_stop_bits, parity_bit, divisor, divaddval, mulval);

    return 0;
}

static int uart_verb_read(struct command_transaction *trans)
{
    uint32_t uart_num 			= comms_argument_parse_uint32_t(trans);
    uint8_t rx_data = uart_read(UART0_NUM);
	comms_response_add_uint8_t(trans, rx_data);

    // TODO: add timeout and/or desired number of bytes to read

    return 0;
}

static int uart_verb_write(struct command_transaction *trans)
{
    uint32_t uart_num 			= comms_argument_parse_uint32_t(trans);
    uint8_t scu_conf_func       = comms_argument_parse_uint8_t(trans);
    uint8_t scu_port            = comms_argument_parse_uint8_t(trans);
    uint8_t scu_pin             = comms_argument_parse_uint8_t(trans);
    
    switch(uart_num) {
        case 0:
            uart_num = UART0_NUM;
            break;
        case 1:
            uart_num = UART1_NUM;
            break;
        case 2:
            uart_num = UART2_NUM;
            break;
        case 3:
            uart_num = UART3_NUM;
            break;
    }

    // If we can't get a hold on the given pin.
	if (!pin_ensure_reservation(scu_port, scu_pin, CLASS_NUMBER_SELF)) {
		pr_warning("uart: couldn't reserve busy pin SCU%d[%d]!\n", scu_port, scu_pin);
		return EBUSY;
	}

    while (comms_argument_data_remaining(trans)) {
        uint8_t tx_byte         = comms_argument_parse_uint8_t(trans);
        // TODO: fix port/pin tuple for pinmux call
        scu_pinmux(P2_3, SCU_UART_RX_TX | scu_conf_func);
        uart_write(uart_num, tx_byte);
    }

    return 0;
}

/**
 * Verbs for the firmware API.
 */
static struct comms_verb _verbs[] = {
		{ .name = "init", .handler = uart_verb_init,
			.in_signature = "<IBBBH", .out_signature = "",
			.in_param_names = "uart_num, num_of_data_bits, stop_bit, parity_bit, baud",
			.doc = "Initialize UART" },
        { .name = "read", .handler = uart_verb_read,
			.in_signature = "<I", .out_signature = "<*B",
			.in_param_names = "uart_num", .out_param_names = "response",
			.doc = "Read data from UART device" },
		{ .name = "write", .handler = uart_verb_write,
			.in_signature = "<IBBB*X", .out_signature = "",
			.in_param_names = "uart_num, scu_func, scu_port, scu_pin, data", .out_param_names = "",
			.doc = "Write data to UART device" },
		{} // Sentinel
};
COMMS_DEFINE_SIMPLE_CLASS(uart, CLASS_NUMBER_SELF, "uart", _verbs,
		"API for UART communication.");

