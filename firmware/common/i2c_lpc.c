/*
 * This file is part of GreatFET
 */

#include "i2c_lpc.h"

#include <libopencm3/lpc43xx/i2c.h>

// #include "usb_api_spi.h"
#include "usb_queue.h"

#include <stddef.h>
#include <greatfet_core.h>
#include <i2c_bus.h>
#include <i2c.h>

/* FIXME return i2c0 status from each function */

void i2c_lpc_start(i2c_bus_t* const bus, uint16_t _duty_cycle_count) {
	uint16_t duty_cycle_count = _duty_cycle_count;

	const uint32_t port = (uint32_t)bus->obj;
	i2c_init(port, duty_cycle_count);
}

void i2c_lpc_stop(i2c_bus_t* const bus) {
	const uint32_t port = (uint32_t)bus->obj;
	i2c_disable(port);
}

uint8_t i2c_lpc_transfer(i2c_bus_t* const bus,
	const uint_fast8_t slave_address,
	const uint8_t* const data_tx, const size_t count_tx,
	uint8_t* const data_rx, const size_t count_rx
) {
	const uint32_t port = (uint32_t)bus->obj;
	size_t i;
	bool ack = false;
	uint8_t status = 0;
	if (data_tx && (count_tx > 0)) {
		i2c_tx_start(port);
		i2c_tx_byte(port, (slave_address << 1) | I2C_WRITE);
		for(i=0; i<count_tx; i++) {
			i2c_tx_byte(port, data_tx[i]);
		}
	}

	if (data_rx && (count_rx > 0)) {
		i2c_tx_start(port);
		i2c_tx_byte(port, (slave_address << 1) | I2C_READ);
		status = I2C0_STAT;
		// This only works here because only the Setup + ACK/NAK has happened so far
		// anything but 0x40 here is bad 
		// if (I2C0_STAT == 0x40) {
		// 	led_on(LED2);
		// } else {
		// 	led_on(LED3);
		// }

		for(i=0; i<count_rx; i++) {
			/* ACK each byte except the last */
			ack = (i!=count_rx-1);
			data_rx[i] = i2c_rx_byte(port, ack);
		}
	}
	i2c_stop(port);

	return status;
}