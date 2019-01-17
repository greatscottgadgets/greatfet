/*
 * This file is part of GreatFET
 */

#ifndef __I2C_LPC_H__
#define __I2C_LPC_H__

#include <stdint.h>
#include <stddef.h>

#include "i2c_bus.h"

void i2c_lpc_start(i2c_bus_t* const bus, uint16_t duty_cycle_count);
void i2c_lpc_stop(i2c_bus_t* const bus);
uint8_t i2c_lpc_read(i2c_bus_t* const bus,
	const uint_fast8_t slave_address,
	uint8_t* const data_rx, const size_t count_rx
);
uint8_t i2c_lpc_write(i2c_bus_t* const bus,
	const uint_fast8_t slave_address,
	const uint8_t* const data_tx, const size_t count_tx
);
#endif/*__I2C_LPC_H__*/
