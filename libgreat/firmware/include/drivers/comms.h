/*
 * This file is part of libgreat
 *
 * High-level communications API -- used by all devices that wish to speak
 * the standard communications protocol.
 */


#ifndef __LIBGREAT_COMMS_H__
#define __LIBGREAT_COMMS_H__

#include <stdbool.h>
#include <toolchain.h>
#include <stdint.h>

/**
 * Structure composing the objects for a given
 */
struct command_transaction {

	/**
	 * The class number for the given transaciton.
	 */
	uint32_t class_number;

	/**
	 * The verb number for the given transaction.
	 */
	uint32_t verb;

	/*
	 * Pointer to a byte buffer that provides the command's input,
	 * or NULL if no input is being provided.
	 */
	void *data_in;

	/*
	 * The length of the data provided in the data_in buffer.
	 * Must be 0 if data_in is null.
	 */
	uint32_t data_in_length;

	/*
	 * Pointer to a byte buffer that accepts the command's output.
	 * or NULL if no output is being provided.
	 */
    void *data_out;

	/*
	 * The maximum response length that the device is willing to accept.
	 * This must be 0 if data_out is null.
	 */
	uint32_t data_out_max_length;

	/*
	 * Out argument that accepts the amount of data to be returned
	 * by the given command. Must be <= data_out_max_length.
	 */
	uint32_t data_out_length;


    /**
     * State tracking for parsing of command responses.
     * These variables are used internally and are essentially private.
     */
    void *data_in_position;
    void *data_out_position;
};



/**
 * Callback types for commands registered by communications backends.
 *
 * @param trans The transcation data for the given transaction.
 *	See the struct definition for each field.
 *
 * @return 0 if the operation went successfully, or an error code on failure.
 *      This will be converted to a protocol-specific response, and the error
 *      code may not be conveyed.
 */
typedef int (*command_handler_function)(struct command_transaction *trans);

/**
 * Structure describing the various operations that can be performed by a
 * (conceptual) pipe.
 */
struct comms_pipe_ops {

    /**
     * Handle data being recieved from the host.
     *
     * @param verb -- The verb, if this is a shared pipe. For a dedicate pipe,
     *      this value is always zero.
     * @param data_in -- Pointer to the block of data recieved.
     * @param length -- The length of the data recieved.
     *
     * @return 0 on success, or an error code on failure
     *      Not all tranports will respect error codes, for now.
     */
    int (*handle_data_in)(uint32_t verb, void *data_in, uint32_t length);


    /**
     * Handles an indication that the host is ready to recieve data.
     *
     * @param verb -- The verb, if this is a shared pipe. For a dedicate pipe,
     *      this value is always zero.
     */
    void (*handle_host_ready_for_data)(uint32_t verb);


    /**
     * Handles completion of a transmission on a pipe. This callback gives
     * us the ability to free data after use, if desired.
     */
    int (*handle_data_out_complete)(void *data, uint32_t length);
};


/**
 * Data structure that describes a standard communication verb.
 */
struct comms_verb {

	/* The number of the verb to be accepted. */
	uint32_t verb_number;

	/* The name of the verb; consumed by the host API. */
	char *name;

	/* The command handler -- must be non-NULL for
	 * any non-sentinel (termiatnor) verb. */
	command_handler_function handler;

};


/**
 * Data structure that describes a libgreat class.
 */
struct comms_class {

    /**
     * The number for the provided class. These should be reserved
     * on the relevant project's wiki.
     */
    uint32_t class_number;

	/**
	 * Printable name for the class.
	 * Used mostly for debug output.
	 */
	char *name;

	/**
	 * A function that will accept any commands issued to this class.
	 * If this is NULL, the command_verb list will be used to automatically
	 * select a verb function.
	 */
	command_handler_function command_handler;


	/**
	 * Pointer to an array of verb objects,
	 * terminated by a verb with a handler of NULL.
	 *
	 * Used only if the command_handler is NULL.
	 */
	struct comms_verb *command_verbs;


	/**
	 *  Forms a singly-linked list of comm classes.
	 *
	 *	Pointer to the the next communciations class in the list,
	 *	or NULL if no classes remain.
	 */
	struct comms_class *next;

	/** TODO: pipe objects */
};


/**
 * Object describing a communications pipe.
 */
struct comms_pipe {

};



/**
 * Registers a given class for use with libgreat; which implicitly provides it
 * with an ability to handle commands.
 *
 * @param comms_class The comms class to be registered. This object will continue
 *	to be held indefinition, so it must be permanently allocated.
 */
void comms_register_class(struct comms_class *comms_class);


/**
 * Registers a pipe to be provided for a given class, which allows
 * bulk bidirectional communications.
 *
 * @param class_number -- The number for the class for which the pipe is
 *      to be associated. This must have already been registered with
 *      register_class.
 * @param flags -- Flags describing how this pipe is to operate. TBD.
 * @param ops -- A structure defining the operations this pipe supports.
 *
 * @returns a comms_pipe object on success; or NULL on failure
 */
struct comms_pipe *comms_register_pipe(struct comms_class *owning_class,
        uint32_t flags, struct comms_pipe_ops ops);


/**
 * Transmits data on a given communications pipe.
 *
 * @param pipe The pipe on which to transmit.
 * @param data Buffer storing the data to be transmitted.
 * @param length The length of the data to be transmitted.
 */
int comms_send_on_pipe(struct comms_pipe *pipe, void *data, uint32_t length);


/**
 * @return True iff the given comms pipe is ready for data transmission.
 */
bool comms_pipe_ready(struct comms_pipe *pipe);


/**
 * Macros that allow us to avoid boilerplate declarations.
 */

/* Registers a given comms_class_t for use. */
#define COMMS_PROVIDE_CLASS(defined_name) \
	void defined_name##__auto_initializer(void) { comms_register_class(&defined_name); }; \
	CALL_ON_INIT(defined_name##__auto_initializer)

/* Defines a comms_class_t and registers it for use. */
#define COMMS_DEFINE_SIMPLE_CLASS(defined_name, number, string, verbs) \
	struct comms_class defined_name##__object_ = { \
		.name = string, \
		.class_number = number, \
		.command_verbs = verbs, \
	}; \
	COMMS_PROVIDE_CLASS(defined_name##__object_)


/**
 * Simple functions that help us to parse arguments and respond with data.
 */ 
#define COMMS_DECLARE_RESPONSE_HANDLER(type) \
	void *comms_response_add_##type(struct command_transaction *trans, type response)
#define COMMS_DECLARE_ARGUMENT_HANDLER(type) \
	type comms_argument_parse_##type(struct command_transaction *trans);

/**
 * Convenience functions -- declared in util.h
 */
COMMS_DECLARE_RESPONSE_HANDLER(uint8_t);
COMMS_DECLARE_RESPONSE_HANDLER(uint16_t);
COMMS_DECLARE_RESPONSE_HANDLER(uint32_t);
COMMS_DECLARE_RESPONSE_HANDLER(int8_t);
COMMS_DECLARE_RESPONSE_HANDLER(int16_t);
COMMS_DECLARE_RESPONSE_HANDLER(int32_t);
COMMS_DECLARE_ARGUMENT_HANDLER(uint8_t);
COMMS_DECLARE_ARGUMENT_HANDLER(uint16_t);
COMMS_DECLARE_ARGUMENT_HANDLER(uint32_t);
COMMS_DECLARE_ARGUMENT_HANDLER(int8_t);
COMMS_DECLARE_ARGUMENT_HANDLER(int16_t);
COMMS_DECLARE_ARGUMENT_HANDLER(int32_t);

/**
 * Adds a string to the communications response.
 */
void *comms_response_add_string(struct command_transaction *trans, char *response);

#endif
