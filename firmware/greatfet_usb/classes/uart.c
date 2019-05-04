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

static int uart_verb_init(struct command_transaction *trans)
{
    uint32_t uart_num 			= comms_argument_parse_uint32_t(trans);
    uint8_t number_of_data_bits	= comms_argument_parse_uint8_t(trans);
    uint8_t number_of_stop_bits = comms_argument_parse_uint8_t(trans);
    uint8_t parity_bit			= comms_argument_parse_uint8_t(trans);
    uint16_t divisor            = comms_argument_parse_uint16_t(trans);
    uint8_t divaddval           = comms_argument_parse_uint8_t(trans);
    uint8_t mulval              = comms_argument_parse_uint8_t(trans);

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

    uart_init(uart_num, number_of_data_bits, number_of_stop_bits, parity_bit, divisor, divaddval, mulval);

    return 0;
}

static int uart_verb_read(struct command_transaction *trans)
{
    uint32_t uart_num 			= comms_argument_parse_uint32_t(trans);
    uint8_t scu_conf_func       = comms_argument_parse_uint8_t(trans);
    uint8_t scu_group           = comms_argument_parse_uint8_t(trans);
    uint8_t scu_pin             = comms_argument_parse_uint8_t(trans);
    uint16_t rx_length 		    = comms_argument_parse_uint16_t(trans);
	uint8_t *uart_rx_buffer 	= comms_response_reserve_space(trans, rx_length);

    uint32_t scu_group_pin = 0;
    
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

    switch(scu_group) {
        case 1:
            scu_group_pin += PIN_GROUP1;
            break;
        case 2:
            scu_group_pin += PIN_GROUP2;
            break;
        case 3:
            scu_group_pin += PIN_GROUP3;
            break;
        case 4:
            scu_group_pin += PIN_GROUP4;
            break;
        case 5:
            scu_group_pin += PIN_GROUP5;
            break;
        case 6:
            scu_group_pin += PIN_GROUP6;
            break;
        case 7:
            scu_group_pin += PIN_GROUP7;
            break;
        case 9:
            scu_group_pin += PIN_GROUP9;
            break;
    }

    switch(scu_pin) {
        case 0:
            scu_group_pin += PIN0;
            break;
        case 1:
            scu_group_pin += PIN1;
            break;
        case 2:
            scu_group_pin += PIN2;
            break;
        case 3:
            scu_group_pin += PIN3;
            break;
        case 4:
            scu_group_pin += PIN4;
            break;
        case 5:
            scu_group_pin += PIN5;
            break;
        case 6:
            scu_group_pin += PIN6;
            break;
        case 13:
            scu_group_pin += PIN13;
            break;
        case 14:
            scu_group_pin += PIN14;
            break;
        case 15:
            scu_group_pin += PIN15;
            break;
        case 16:
            scu_group_pin += PIN16;
            break;
    }

    // If we can't get a hold on the given pin.
	if (!pin_ensure_reservation(scu_group, scu_pin, CLASS_NUMBER_SELF)) {
		pr_warning("uart: couldn't reserve busy pin SCU%d[%d]!\n", scu_group, scu_pin);
		return EBUSY;
	}

    scu_pinmux(scu_group_pin, SCU_UART_RX_TX | scu_conf_func);
    uart_error_t *error;  

    for (int i = 0; i < rx_length; i++) {
        uart_rx_buffer[i] = uart_read_timeout(uart_num, 20000, error);
    }

    return 0;
}

static int uart_verb_write(struct command_transaction *trans)
{
    uint32_t uart_num 			= comms_argument_parse_uint32_t(trans);
    uint8_t scu_conf_func       = comms_argument_parse_uint8_t(trans);
    uint8_t scu_group           = comms_argument_parse_uint8_t(trans);
    uint8_t scu_pin             = comms_argument_parse_uint8_t(trans);

    uint32_t scu_group_pin = 0;
    
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

    switch(scu_group) {
        case 1:
            scu_group_pin += PIN_GROUP1;
            break;
        case 2:
            scu_group_pin += PIN_GROUP2;
            break;
        case 3:
            scu_group_pin += PIN_GROUP3;
            break;
        case 4:
            scu_group_pin += PIN_GROUP4;
            break;
        case 5:
            scu_group_pin += PIN_GROUP5;
            break;
        case 6:
            scu_group_pin += PIN_GROUP6;
            break;
        case 7:
            scu_group_pin += PIN_GROUP7;
            break;
        case 9:
            scu_group_pin += PIN_GROUP9;
            break;
    }

    switch(scu_pin) {
        case 0:
            scu_group_pin += PIN0;
            break;
        case 1:
            scu_group_pin += PIN1;
            break;
        case 2:
            scu_group_pin += PIN2;
            break;
        case 3:
            scu_group_pin += PIN3;
            break;
        case 4:
            scu_group_pin += PIN4;
            break;
        case 5:
            scu_group_pin += PIN5;
            break;
        case 6:
            scu_group_pin += PIN6;
            break;
        case 13:
            scu_group_pin += PIN13;
            break;
        case 14:
            scu_group_pin += PIN14;
            break;
        case 15:
            scu_group_pin += PIN15;
            break;
        case 16:
            scu_group_pin += PIN16;
            break;
    }

    // If we can't get a hold on the given pin.
	if (!pin_ensure_reservation(scu_group, scu_pin, CLASS_NUMBER_SELF)) {
		pr_warning("uart: couldn't reserve busy pin SCU%d[%d]!\n", scu_group, scu_pin);
		return EBUSY;
	}

    while (comms_argument_data_remaining(trans)) {
        uint8_t tx_byte         = comms_argument_parse_uint8_t(trans);
        scu_pinmux(scu_group_pin, SCU_UART_RX_TX | scu_conf_func);
        uart_write(uart_num, tx_byte);
    }

    return 0;
}

/**
 * Verbs for the firmware API.
 */
static struct comms_verb _verbs[] = {
		{ .name = "init", .handler = uart_verb_init,
			.in_signature = "<IBBBHBB", .out_signature = "",
			.in_param_names = "uart_num, num_of_data_bits, stop_bit, parity_bit, baud",
			.doc = "Initialize UART" },
        { .name = "read", .handler = uart_verb_read,
			.in_signature = "<IBBBH", .out_signature = "<*B",
			.in_param_names = "uart_num, scu_func, scu_port, scu_pin, rx_length", .out_param_names = "response",
			.doc = "Read data from UART device" },
		{ .name = "write", .handler = uart_verb_write,
			.in_signature = "<IBBB*X", .out_signature = "",
			.in_param_names = "uart_num, scu_func, scu_port, scu_pin, data", .out_param_names = "",
			.doc = "Write data to UART device" },
		{} // Sentinel
};
COMMS_DEFINE_SIMPLE_CLASS(uart, CLASS_NUMBER_SELF, "uart", _verbs,
		"API for UART communication.");

