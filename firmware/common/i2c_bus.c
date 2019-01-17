/*
 * This file is part of GreatFET
 */

#include "i2c_bus.h"

void i2c_bus_start(i2c_bus_t* const bus, const uint16_t config) {
	bus->start(bus, config);
}

void i2c_bus_stop(i2c_bus_t* const bus) {
	bus->stop(bus);
}

uint8_t i2c_bus_read(
	i2c_bus_t* const bus,
	const uint_fast8_t slave_address,
	uint8_t* const rx, const size_t rx_count
) {
	return bus->read(bus, slave_address, rx, rx_count);
}

uint8_t i2c_bus_write(
	i2c_bus_t* const bus,
	const uint_fast8_t slave_address,
	const uint8_t* const tx, const size_t tx_count
) {
	return bus->write(bus, slave_address, tx, tx_count);
}