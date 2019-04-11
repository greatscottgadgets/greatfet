/*
 * This file is part of GreatFET
 */
#include <libopencm3/lpc43xx/uart.h>

#include <stddef.h>
#include <greatfet_core.h>


// void i2c_lpc_start(i2c_bus_t* const bus, uint16_t _duty_cycle_count) {
// 	uint16_t duty_cycle_count = _duty_cycle_count;

// 	const uint32_t port = (uint32_t)bus->obj;
// 	i2c_init(port, duty_cycle_count);
// }

void uart_lpc_start() {
    uint16_t divisor = 2;   // arbitrary value
    uint8_t divaddval = 4;  // arbitrary value
    uint8_t mulval = 9;     // arbitrary value
    uart_init(UART0, UART_DATABIT_8, UART_STOPBIT_1, UART_PARITY_EVEN, divisor, divaddval, mulval);
}

// void i2c_lpc_stop(i2c_bus_t* const bus) {
// 	const uint32_t port = (uint32_t)bus->obj;
// 	i2c_disable(port);
// }


// uint8_t i2c_lpc_read(i2c_bus_t* const bus,
// 	const uint_fast8_t slave_address,
// 	uint8_t* const data_rx, const size_t count_rx
// ) {
// 	const uint32_t port = (uint32_t)bus->obj;
// 	size_t i;
// 	bool ack = false;
// 	uint8_t status = 0;

// 	i2c_tx_start(port);
// 	i2c_tx_byte(port, (slave_address << 1) | I2C_READ);
// 	status = I2C0_STAT;

// 	for(i=0; i<count_rx; i++) {
// 		/* ACK each byte except the last */
// 		ack = (i!=count_rx-1);
// 		data_rx[i] = i2c_rx_byte(port, ack);
// 	}
// 	i2c_stop(port);

// 	return status;
// }

uint8_t uart_lpc_read() {
    
}

// uint8_t i2c_lpc_write(i2c_bus_t* const bus,
// 	const uint_fast8_t slave_address,
// 	const uint8_t* const data_tx, const size_t count_tx
// ) {
// 	const uint32_t port = (uint32_t)bus->obj;
// 	size_t i;
// 	uint8_t status = 0;

// 	i2c_tx_start(port);
// 	i2c_tx_byte(port, (slave_address << 1) | I2C_WRITE);
// 	status = I2C0_STAT;
// 	for(i=0; i<count_tx; i++) {
// 		i2c_tx_byte(port, data_tx[i]);
// 	}
// 	i2c_stop(port);

// 	return status;
// }