# Copyright 2012 Jared Boone <jared@sharebrained.com>
#
# This file is part of GreatFET.
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 2, or (at your option)
# any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; see the file COPYING.  If not, write to
# the Free Software Foundation, Inc., 51 Franklin Street,
# Boston, MA 02110-1301, USA.
#

cmake_minimum_required(VERSION 2.8.9)
set(CMAKE_TOOLCHAIN_FILE ../cmake/toolchain-arm-cortex-m.cmake)

project(greatfet_usb)

include(../cmake/greatfet-common.cmake)

set(SRC_M4
	greatfet_usb.c
	usb_bulk_buffer.c
	"${PATH_GREATFET_FIRMWARE_COMMON}/usb.c"
	"${PATH_GREATFET_FIRMWARE_COMMON}/usb_request.c"
	"${PATH_GREATFET_FIRMWARE_COMMON}/usb_standard_request.c"
	usb_descriptor.c
	usb_device.c
	usb_endpoint.c
	usb_api_board.c
	usb_api_spiflash.c
	usb_api_spi.c
	usb_api_adc.c
	usb_api_i2c.c
	usb_api_gpio.c
	usb_api_logic_analyzer.c
	usb_api_sdir.c
    usb_api_greatdancer.c
	sgpio_isr.c
	"${PATH_GREATFET_FIRMWARE_COMMON}/usb_queue.c"
	"${PATH_GREATFET_FIRMWARE_COMMON}/fault_handler.c"
	"${PATH_GREATFET_FIRMWARE_COMMON}/rom_iap.c"
	"${PATH_GREATFET_FIRMWARE_COMMON}/spi_bus.c"
)

DeclareTargets()
