/*
 * This file is part of GreatFET
 */

#include "usb_device.h"

#include <drivers/usb/lpc43xx/usb_type.h>

#include "usb_descriptor.h"

usb_configuration_t usb0_configuration_high_speed = {
	.number = 1,
	.speed = USB_SPEED_HIGH,
	.descriptor = usb0_descriptor_configuration_high_speed,
};

usb_configuration_t usb0_configuration_full_speed = {
	.number = 1,
	.speed = USB_SPEED_FULL,
	.descriptor = usb0_descriptor_configuration_full_speed,
};

usb_configuration_t* usb0_configurations[] = {
	&usb0_configuration_high_speed,
	&usb0_configuration_full_speed,
	0,
};

/* USB1 Experiment */
usb_configuration_t usb1_configuration_full_speed = {
	.number = 1,
	.speed = USB_SPEED_FULL,
	.descriptor = usb1_descriptor_configuration_full_speed,
};

usb_configuration_t* usb1_configurations[] = {
	&usb1_configuration_full_speed,
	0,
};

usb_peripheral_t usb_peripherals[] = {
	{
		.descriptor = usb0_descriptor_device,
		.descriptor_strings = usb0_descriptor_strings,
		.qualifier_descriptor = usb0_descriptor_device_qualifier,
		.configurations = &usb0_configurations,
		.configuration = 0,
		.controller = 0,
	},
	{
		.descriptor = usb1_descriptor_device,
		.descriptor_strings = usb1_descriptor_strings,
		.qualifier_descriptor = usb1_descriptor_device_qualifier,
		.configurations = &usb1_configurations,
		.configuration = 0,
		.controller = 1,
	}
};
