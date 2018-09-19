/*
 * This file is part of GreatFET
 */

#include "usb_api_DS18B20.h"
#include <drivers/usb/lpc43xx/usb.h>
#include <drivers/usb/lpc43xx/usb_queue.h>
#include "usb_endpoint.h"

#include <stddef.h>
#include <greatfet_core.h>
#include <pins.h>
#include <gpio_lpc.h>
#include <one_wire.h>

#include <libopencm3/lpc43xx/scu.h>



int16_t read_temperature(void)
{
	int i;
	uint8_t data[9];
    one_wire_init();
	one_wire_init_target();
    one_wire_write(0xCC); // Skip ROM command
    one_wire_write(0x44); // Read temperature
    one_wire_delay_us(750000); // 750 ms for 12 bit temperature conversion
	one_wire_init_target();
    one_wire_write(0xCC); // Skip ROM command
    one_wire_write(0xBE); // Read scratchpad area
    one_wire_delay_us(750000);
	for(i=0; i<9; i++) {
		// scratchpad is 9 bytes
		data[i] = one_wire_read();
	}
	one_wire_init_target();
	return data[1] << 8 | data[0];
}

usb_request_status_t usb_vendor_request_DS18B20_read(
	usb_endpoint_t* const endpoint, const usb_transfer_stage_t stage)
{
	scu_pinmux(SCU_PINMUX_GPIO5_8, SCU_GPIO_PUP | SCU_CONF_FUNCTION4);
	int16_t temperature;
	if (stage == USB_TRANSFER_STAGE_SETUP) {
		temperature = read_temperature();
		usb_transfer_schedule_block(endpoint->in, &temperature, 2, NULL, NULL);
		usb_transfer_schedule_ack(endpoint->out);
	}
	return USB_REQUEST_STATUS_OK;
}
