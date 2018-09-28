/*
 * This file is part of libgreat
 *
 * High-level communications API -- convenience functions.
 */

#include <string.h>
#include <stddef.h>
#include <stdint.h>
#include <drivers/comms.h>
#include <debug.h>

/**
 * Dark metaprogramming magic.
 * If only C had the Rust-like macros. :)
 */
#define COMMS_DEFINE_RESPONSE_HANDLER(type) \
	void *comms_response_add_##type(struct command_transaction *trans, type response) \
	{ \
		type *target = trans->data_out_position; \
		\
		if (trans->data_out_max_length - trans->data_out_length < sizeof(type)) \
			return trans->data_out_position; \
		\
		trans->data_out_length += sizeof(type); \
		*target = response; \
		\
		++target; \
		trans->data_out_position = target; \
		return target; \
	} 

#define COMMS_DEFINE_ARGUMENT_HANDLER(type) \
	type comms_argument_parse_##type(struct command_transaction *trans) \
	{ \
		type *target = trans->data_in_position; \
		type value = *target; \
		uintptr_t position = (uintptr_t)trans->data_in_position - (uintptr_t)trans->data_in; \
		\
		if (position + sizeof(type) > trans->data_in_length) { \
			pr_error("ERROR: command overread handling class %d / verb %d! \n", trans->class_number, trans->verb); \
			return (type)0; \
		} \
		\
		++target; \
		trans->data_in_position = target; \
		\
		return value; \
	} 


/** Quick response handling functions. */
COMMS_DEFINE_RESPONSE_HANDLER(uint8_t);
COMMS_DEFINE_RESPONSE_HANDLER(uint16_t);
COMMS_DEFINE_RESPONSE_HANDLER(uint32_t);
COMMS_DEFINE_RESPONSE_HANDLER(int8_t);
COMMS_DEFINE_RESPONSE_HANDLER(int16_t);
COMMS_DEFINE_RESPONSE_HANDLER(int32_t);

/** Quick argument read functions. */
COMMS_DEFINE_ARGUMENT_HANDLER(uint8_t);
COMMS_DEFINE_ARGUMENT_HANDLER(uint16_t);
COMMS_DEFINE_ARGUMENT_HANDLER(uint32_t);
COMMS_DEFINE_ARGUMENT_HANDLER(int8_t);
COMMS_DEFINE_ARGUMENT_HANDLER(int16_t);
COMMS_DEFINE_ARGUMENT_HANDLER(int32_t);

void *comms_response_add_string(struct command_transaction *trans, char *response)
{
	// Copy the string into our response buffer.
	size_t length = strlen(response);
    size_t length_left = (trans->data_out_max_length - trans->data_out_length);

    uint8_t *out_pointer = trans->data_out_position;

    // If we can't fit the relevant repsonse, fail out.
    if (length > length_left) {
			pr_error("ERROR: command overwrite responding to class %d / verb %d! \n", trans->class_number, trans->verb); \
            return out_pointer;
    }

	strlcpy(trans->data_out_position, response, length_left);

	// Truncate the length to the maximum response.
	if (length > length_left)
		length = length_left;

	// Store the actual length transmitted, and advance our pointer.
	trans->data_out_length = length;
	out_pointer += length;

    trans->data_out_position = response;
	return response;
}
