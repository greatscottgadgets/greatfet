/*
 * This file is part of GreatFET
 */

#include "spi_bus.h"

void spi_bus_start(spi_target_t* target, const void* const config) {
	target->bus->start(target, config);
}

void spi_bus_stop(spi_bus_t* const bus) {
	bus->stop(bus);
}

void spi_bus_transfer(spi_target_t* target, void* const data, const size_t count) {
	target->bus->transfer(target, data, count);
}

void spi_bus_transfer_data(spi_target_t* target, void* const data, const size_t count) {
	target->bus->transfer_data(target, data, count);
}

void spi_bus_transfer_gather(spi_target_t* target, const spi_transfer_t* const transfers, const size_t count) {
	target->bus->transfer_gather(target, transfers, count);
}


void spi_bus_transfer_gather_partial(spi_target_t* target, const spi_transfer_t* const transfers, const size_t count) {
	target->bus->transfer_gather_partial(target, transfers, count);
}
