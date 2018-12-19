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
 *
 * FIXME: these should set properties on trans on failure
 */
#define COMMS_DEFINE_RESPONSE_HANDLER(type) \
	void *comms_response_add_##type(struct command_transaction *trans, type response) \
	{ \
		type *target = trans->data_out_position; \
		\
		if ((trans->data_out_max_length - trans->data_out_length) < sizeof(type)) {\
			pr_comms_error(trans, "not enough space to include %s response\n", #type); \
			trans->data_out_status |= COMMS_PARSE_OVERRUN; \
			return trans->data_out_position; \
		} \
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
		\
		if (sizeof(type) > trans->data_in_remaining) { \
			pr_comms_error(trans, "not enough data provided to read %s response (%d byte(s) left)\n", #type, \
					trans->data_in_remaining); \
			trans->data_in_status |= COMMS_PARSE_UNDERRUN; \
			return (type)0; \
		} \
		\
		++target; \
		trans->data_in_remaining -= sizeof(type); \
		trans->data_in_position = target; \
		\
		return value; \
	}


#define COMMS_DEFINE_HELPERS(type) \
	COMMS_DEFINE_RESPONSE_HANDLER(type); \
	COMMS_DEFINE_ARGUMENT_HANDLER(type)

/** Quick argument/response handling functions. */
COMMS_DEFINE_HELPERS(uint8_t);
COMMS_DEFINE_HELPERS(uint16_t);
COMMS_DEFINE_HELPERS(uint32_t);
COMMS_DEFINE_HELPERS(int8_t);
COMMS_DEFINE_HELPERS(int16_t);
COMMS_DEFINE_HELPERS(int32_t);
COMMS_DEFINE_HELPERS(_Bool);

void *comms_response_add_string(struct command_transaction *trans, char const *const response)
{
	// Copy the string into our response buffer.
	size_t length = strlen(response) + 1;
	size_t length_left = (trans->data_out_max_length - trans->data_out_length);

	uint8_t *out_pointer = trans->data_out_position;

	// If we can't fit the relevant repsonse, fail out.
	if (length > length_left) {
			pr_comms_error(trans, "not enough space to add target string!\n");
			trans->data_out_status = COMMS_PARSE_OVERRUN;
			return out_pointer;
	}

	strlcpy(trans->data_out_position, response, length_left);

	// Truncate the length to the maximum response.
	if (length > length_left)
		length = length_left;

	// Store the actual length transmitted, and advance our pointer.
	trans->data_out_length = length;
	out_pointer += length;

	trans->data_out_position = (void *)response;
	return (void *)response;
}


/**
 * Grabs a reference to a chunk of up to max_length from the argument buffer.
 *
 * @param trans The associated transaction.
 * @param max_length The maximum amount to read; or -1 for the remainder of the bfufer.
 * @param out_length Out argument; accepts the actual amount read.
 * @return A pointer to a buffer within the transaction that contains the relevant data.
 */
void *comms_argument_read_buffer(struct command_transaction *trans,
		uint32_t max_length, uint32_t *out_length)
{
	void *start_pointer = trans->data_in_position;
	uint8_t *end_pointer = start_pointer;
	uint32_t length = trans->data_in_remaining;

	// If our maximum length is less than the data available, truncate.
	if (max_length < length) {
		length = max_length;
	}

	// Advance within the buffer accordingly.
	end_pointer += length;
	trans->data_in_remaining -= length;
	trans->data_in_position = end_pointer;

	if (out_length) {
		*out_length = length;
	}
	return start_pointer;
}


/**
 * Reserves a buffer of the provided size in the data output buffer.
 *
 * @param trans The associated transaction.
 * @param size The amount of space to reserve.
 *
 * @return A pointer to the buffer that can be used for the relevant response,
 *		or NULL if the relevant amount of space could not be reserved.
 */
void *comms_response_reserve_space(struct command_transaction *trans, uint32_t size)
{
	void *start_pointer = trans->data_out_position;
	uint8_t *end_pointer = start_pointer;
	uint32_t available_length = trans->data_out_max_length - trans->data_out_length;

	if (size > available_length) {
		pr_comms_error(trans, "not enough space to reserve %d requested bytes\n", size);
		trans->data_out_status = COMMS_PARSE_OVERRUN;
		return NULL;
	}

	end_pointer += size;
	trans->data_out_length += size;
	trans->data_out_position = end_pointer;

	return start_pointer;
}


/**
 * Adds a collection of raw bytes to the response.
 *
 * @param trans The associated transaction.
 * @param data Data buffer to be transmitted.
 * @param length The total amount of data from the buffer to include in the response.
 *
 * @return A pointer to the buffer used in the response,
 *      or NULL if the relevant amount of space could not be reserved.
 */
void *comms_response_add_raw(struct command_transaction *trans, void *data, uint32_t length)
{
	// Allocate space in the response for the given buffer.
	void *buffer = comms_response_reserve_space(trans, length);

	// If we weren't able to allocate a buffer, fail out.
	// Note that comms_response_reserve_space has already handled errors for us.
	if (!buffer) {
		return buffer;
	}

	// Copy the provided data into the reserved buffer.
	memcpy(buffer, data, length);
	return buffer;
}
