/*
 * This file is part of GreatFET
 *
 * GPIO Interrupt support.
 */

#include "gpio_int.h"

// Maps IRQs to their interrupts.
static const int irq_for_interrupt[] = {
  NVIC_PIN_INT0_IRQ,
  NVIC_PIN_INT1_IRQ,
  NVIC_PIN_INT2_IRQ,
  NVIC_PIN_INT3_IRQ,
  NVIC_PIN_INT4_IRQ,
  NVIC_PIN_INT5_IRQ,
  NVIC_PIN_INT6_IRQ,
  NVIC_PIN_INT7_IRQ,
};

/**
 * Configures a GPIO pin change interrupt in the NVIC, but does not configure
 * the it in the SCU or GPIO interrupt block.
 *
 * @param interrupt_number The GPIO INT#, from 0-7.
 * @param isr The interrupt service routine that will be invoked when the IRQ is raised.
 * @param interrupt_priority The interrupt priority. Takes the same format as nvic_set_priority.
 */
static void _nvic_set_up_pin_change_interrupt(int interrupt_number,
  vector_table_entry_t isr, int interrupt_priority)
{
    // Determine the NVIC IRQ line tied to our GPIO interrupt.
    uint8_t irq = irq_for_interrupt[interrupt_number];

    // Set the location of our interrupt handler, and the interrupt priority.
    vector_table.irq[irq] = isr;
    nvic_set_priority(irq, interrupt_priority);
}

/**
 * Routes a GPIO pin through the SCU to the GPIO interrupt block.
 *
 * @param interrupt_number The input in the interrupt block to which the GPIO should be routed.
 * @param port_number The number of the port whose signal is to be routed.
 * @param pin_number The pin number of the signal to be routed.
 */
static void _route_gpio_to_interrupt_block(int interrupt_number,
    int port_number, int pin_number)
{
    uint32_t offset_into_register;
    volatile uint32_t *interrupt_select_register;
    uint32_t value_to_write;

    // Sanitize the port and pin numbers, ensuring they don't touch
    // unintended parts of the register.
    port_number &= 0x0F;
    pin_number  &= 0x0F;

    // Determine the location of the SCU_PINTSEL registers that handle routing of
    // GPIO signals to the GPIO interrupt block.
    interrupt_select_register = (interrupt_number > 3) ? &SCU_PINTSEL1 : &SCU_PINTSEL0;
    offset_into_register = (interrupt_number % 4) * 8;

    // Figure out the value that will need to be written into the given register...
    value_to_write = (port_number << 4) | pin_number;

    // ... and write that into the given register section.
    *interrupt_select_register &= ~(0xFF << offset_into_register);
    *interrupt_select_register |= (value_to_write << offset_into_register);
}

/**
 * Configures a pin change interrupt inside the GPIO interrupt block.
 *
 * @param interrupt_number The interrupt number to configure.
 * @param sensitivity Determines whether the interrupt will be level or edge sensitive.
 */
static void _gpio_set_up_pin_change_interrupt(int interrupt_number,
    gpio_interrupt_sensitivity_t sensitivity)
{
    // Create a mask for configuration of the GPIO interrupt block.
    const uint32_t mask = (1 << interrupt_number);

    // Set the interrupt as either level or edge sensitive.
    if((sensitivity == LEVEL_SENSITIVE_LOW) || (sensitivity == LEVEL_SENSITIVE_HIGH)) {
        GPIO_PIN_INTERRUPT_ISEL &= ~mask; // level sensitive
    } else {
        GPIO_PIN_INTERRUPT_ISEL |= mask; // edge sensitive
    }

    // Enable the various types of interrupt depending on the requested sensitivyt.
    if(sensitivity == EDGE_SENSITIVE_BOTH) {
        GPIO_PIN_INTERRUPT_SIENR = mask; // trigger on rising edges
        GPIO_PIN_INTERRUPT_SIENF = mask; // and also on falling edges
    }
    else if((sensitivity == EDGE_SENSITIVE_RISING) || (sensitivity == LEVEL_SENSITIVE_HIGH)) {
        GPIO_PIN_INTERRUPT_SIENR = mask; // trigger on rising edges / high levels
        GPIO_PIN_INTERRUPT_CIENF = mask; // don't trigger on falling edges / low levels
    }
    else {
        GPIO_PIN_INTERRUPT_CIENR = mask; // don't trigger on rising edges / high levels
        GPIO_PIN_INTERRUPT_SIENF = mask; // trigger on falling edges / low levels
    }
}

/**
 * Configures a (non-group) GPIO pin-change interrupt. This does not inherently set
 *    up the SCU to use the given pin as a GPIO, nor does this set the given GPIO to input mode.
 *    These should likely be done independently before calling this function. Ths function leaves
 *    the relevant interrupt disabled; if you want to use, you'll need to call gpio_interrupt_enable.
 *
 * @param interrupt_number The GPIO INT#, from 0-7. This value has no relation to the GPIO pin,
 *    but instead indicates which of eight equivalent resources will be used.
 * @param port_number The number of the port whose signal is to be routed.
 * @param pin_number The pin number of the signal to be routed.
 * @param sensitivity Determines whether the interrupt will be level or edge sensitive.
 * @param isr The interrupt service routine that will be invoked when the IRQ is raised.
 * @param interrupt_priority The interrupt priority. Takes the same format as nvic_set_priority.
 */
void gpio_interrupt_configure(int gpio_int_number, int port_number,
    int pin_number, gpio_interrupt_sensitivity_t sensitivity,
    vector_table_entry_t isr, int interrupt_priority)
{
    // Ensure the interrupt's not enabled while we change it.
    gpio_interrupt_disable(gpio_int_number);

    // Set the interrupt priority and masking in the NVIC,
    // as well as the interrupt handler.
    _nvic_set_up_pin_change_interrupt(gpio_int_number, isr, interrupt_priority);

    // Set SCU to route pin-change interrupts requests from the
    // relevent pin to the NVIC.
    _route_gpio_to_interrupt_block(gpio_int_number, port_number, pin_number);

    // Set up the interrupt trigger conditions on the GPIO pin itself.
    _gpio_set_up_pin_change_interrupt(gpio_int_number, sensitivity);
}


/**
 * Enables a given GPIO interrupt previously configured e.g. with gpio_interrupt_configure.
 *
 * @param interrupt_number The GPIO INT#, from 0-7. This value has no relation to the GPIO pin,
 *    but instead indicates which of eight equivalent resources will be used.
 */
void gpio_interrupt_enable(int interrupt_number)
{
    // Determine the NVIC IRQ line tied to our GPIO interrupt...
    uint8_t irq = irq_for_interrupt[interrupt_number];

    // ... and enable the interrupt at the requested priority.
    nvic_enable_irq(irq);
}


/**
 * Disables a given GPIO interrupt previously configured e.g. with gpio_interrupt_configure.
 *
 * @param interrupt_number The GPIO INT#, from 0-7. This value has no relation to the GPIO pin,
 *    but instead indicates which of eight equivalent resources will be used.
 */
void gpio_interrupt_disable(int interrupt_number)
{
    // Determine the NVIC IRQ line tied to our GPIO interrupt...
    uint8_t irq = irq_for_interrupt[interrupt_number];

    // ... and enable the interrupt at the requested priority.
    nvic_enable_irq(irq);
}
