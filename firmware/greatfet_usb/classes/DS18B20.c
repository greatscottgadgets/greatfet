/*
 * This file is part of GreatFET
 *
 * DAC functions
 */

#include <drivers/comms.h>
#include <stddef.h>
#include <greatfet_core.h>
#include <one_wire.h>
#include <pins.h>

#define CLASS_NUMBER_SELF (0x108)

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

int dac_verb_read_DS18B20(struct command_transaction *trans)
{
    scu_pinmux(SCU_PINMUX_GPIO5_8, SCU_GPIO_PUP | SCU_CONF_FUNCTION4);
	uint16_t temperature = read_temperature();
    comms_response_add_uint16_t(trans, temperature);

	return 0;
}

/**
 * Verbs for the DS18B20 API.
 */
static struct comms_verb _verbs[] = {
		{ .name = "read_DS18B20", .handler = dac_verb_read_DS18B20,
            .in_signature = "", .out_signature = "<H",
            .in_param_names = "temperature",
            .doc = "Reads the current temperature from a DS18B20 sensor" },
		{} // Sentinel
};
COMMS_DEFINE_SIMPLE_CLASS(DS18B20, CLASS_NUMBER_SELF, "DS18B20", _verbs,
		"API for DS18B20 / One Wire communication.");