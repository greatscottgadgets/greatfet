/*
 * This file is part of GreatFET
 */

#include <stddef.h>
#include <greatfet_core.h>
#include <jtag_msp430.h>
#include <errno.h>
#include <drivers/comms.h>

#define CLASS_NUMBER_SELF (0x10C)


static int jtag_msp430_verb_start(struct command_transaction *trans)
{
	uint8_t jtagid;
	jtag430_check_init();
	jtagid = jtag430_start_reset_halt();
	comms_response_add_uint8_t(trans, jtagid);
	return 0;
}

static int jtag_msp430_verb_stop(struct command_transaction *trans)
{
	(void)trans;
	jtag430_check_init();
    jtag430_stop();
	return 0;
}

static int jtag_msp430_verb_halt_cpu(struct command_transaction *trans)
{
	(void)trans;
	jtag430_check_init();
    jtag430_haltcpu();
	return 0;
}

static int jtag_msp430_verb_release_cpu(struct command_transaction *trans)
{
	(void)trans;
	jtag430_check_init();
    jtag430_releasecpu();
	return 0;
}

static int jtag_msp430_verb_set_instruction_fetch(struct command_transaction *trans)
{
	(void)trans;
	jtag430_check_init();
    jtag430_setinstrfetch();
	return 0;
}

static int jtag_msp430_verb_read_mem(struct command_transaction *trans)
{
	// FIXME: DGS - tidy up the for loop a bit
	uint32_t addr, length, i;
	uint16_t value;
	jtag430_check_init();
	addr = comms_argument_parse_uint32_t(trans);
	length = comms_argument_parse_uint32_t(trans);
	
    for(i = 0; i < length; i += 2) {
		jtag430_resettap();
		value = jtag430_readmem(addr);
		comms_response_add_uint16_t(trans, value);
		addr += 2;
    }
	return 0;
}

static int jtag_msp430_verb_write_mem(struct command_transaction *trans)
{
	uint32_t addr;
	uint16_t value;
	jtag430_check_init();
	addr = comms_argument_parse_uint32_t(trans);
	value = comms_argument_parse_uint16_t(trans);
	
    jtag430_haltcpu();
    jtag430_writemem(addr, value);
    value = jtag430_readmem(addr);
	comms_response_add_uint16_t(trans, value);
	return 0;
}

static int jtag_msp430_verb_write_flash(struct command_transaction *trans)
{
	uint32_t addr, length, i;
	uint16_t *data_to_write;
	jtag430_check_init();
	addr = comms_argument_parse_uint32_t(trans);
	data_to_write = comms_argument_read_buffer(trans, -1, &length);
	
	for(i=0; i < (length>>1); i++) {
		jtag430_writeflash(addr+(i<<1), data_to_write[i]);
		//Reflash if needed.  Try this twice to save grace?
		if(data_to_write[i]!=jtag430_readmem(addr)) {
			jtag430_writeflash(addr+(i<<1), data_to_write[i]);
		}
    }
	comms_response_add_uint16_t(trans, jtag430_readmem(addr));
	return 0;
}

static int jtag_msp430_verb_erase_flash(struct command_transaction *trans)
{
	(void)trans;
	jtag430_check_init();
    jtag430_erase_entire_flash();
	return 0;
}

static int jtag_msp430_verb_erase_info(struct command_transaction *trans)
{
	(void)trans;
	jtag430_check_init();
    jtag430_erase_info();
	return 0;
}

static int jtag_msp430_verb_set_pc(struct command_transaction *trans)
{
	uint16_t value;
	value = comms_argument_parse_uint16_t(trans);
	jtag430_check_init();
    jtag430_haltcpu();
    jtag430_setpc(value);
    jtag430_releasecpu();
	return 0;
}

static int jtag_msp430_verb_set_reg(struct command_transaction *trans)
{
	uint16_t reg, value;
	reg = comms_argument_parse_uint16_t(trans);
	value = comms_argument_parse_uint16_t(trans);
	jtag430_check_init();
    jtag430_setr(reg, value);
	return 0;
}

/*
 * Verbs for the firmware API.
 */
static struct comms_verb _verbs[] = {
		{ .name = "start", .handler = jtag_msp430_verb_start,
			.in_signature = "", .out_signature = "<B",
			.out_param_names = "jtagid",
			.doc = "Start JTAG process." },
		{ .name = "stop", .handler = jtag_msp430_verb_stop,
			.in_signature = "", .out_signature = "",
			.doc = "Stop JTAG process." },
		{ .name = "halt_cpu", .handler = jtag_msp430_verb_halt_cpu,
			.in_signature = "", .out_signature = "",
			.doc = "Halt program execution." },
		{ .name = "release_cpu", .handler = jtag_msp430_verb_release_cpu,
			.in_signature = "", .out_signature = "",
			.doc = "Release control of the CPU, continuing execution." },
		{ .name = "set_instruction_fetch", .handler = jtag_msp430_verb_set_instruction_fetch,
			.in_signature = "", .out_signature = "",
			.doc = "Put CPU in to instruction fetch state (probably)." },
		{ .name = "read_mem", .handler = jtag_msp430_verb_read_mem,
			.in_signature = "<II", .out_signature = "<*H",
			.in_param_names = "address, length", .out_param_names = "response",
			.doc = "Read n words from memory." },
		{ .name = "write_mem", .handler = jtag_msp430_verb_write_mem,
			.in_signature = "<IH", .out_signature = "<H",
			.in_param_names = "address, value", .out_param_names = "value",
			.doc = "Write a 16 bit word to memory." },
		{ .name = "write_flash", .handler = jtag_msp430_verb_write_flash,
			.in_signature = "<I*X", .out_signature = "<H",
			.in_param_names = "address, data", .out_param_names = "first_word",
			.doc = "Write data to flash from a given address." },
		{ .name = "erase_flash", .handler = jtag_msp430_verb_erase_flash,
			.in_signature = "", .out_signature = "",
			.doc = "Erase all flash (except info segment)." },
		{ .name = "erase_info", .handler = jtag_msp430_verb_erase_info,
			.in_signature = "", .out_signature = "",
			.doc = "Erase info flash segment." },
		{ .name = "set_pc", .handler = jtag_msp430_verb_set_pc,
			.in_signature = "<H", .out_signature = "",
			.in_param_names = "value",
			.doc = "Set CPU program counter." },
		{ .name = "set_reg", .handler = jtag_msp430_verb_set_reg,
			.in_signature = "<HH", .out_signature = "",
			.in_param_names = "register, value",
			.doc = "Set a register to the given value." },
		{} // Sentinel
};
COMMS_DEFINE_SIMPLE_CLASS(jtag_msp430, CLASS_NUMBER_SELF, "jtag_msp430", _verbs,
                          "MSP430 specific JTAG functions.");
