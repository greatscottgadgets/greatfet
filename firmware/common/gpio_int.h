/*
 * This file is part of GreatFET
 *
 * GPIO Interrupt support.
 */

#ifndef __GPIO_INT_H__
#define __GPIO_INT_H__

#include <stdint.h>
#include <libopencm3/lpc43xx/scu.h>


// The LPC4330 has eight interrupts.
#define GPIO_NUM_INTERRUPTS 8


/* GPIO pin interrupts */

/* Pin Interrupt Mode register */
#define GPIO_PIN_INTERRUPT_ISEL         MMIO32(GPIO_PIN_INTERRUPT_BASE + 0x000)

/* Pin interrupt level (rising edge) interrupt enable register */
#define GPIO_PIN_INTERRUPT_IENR         MMIO32(GPIO_PIN_INTERRUPT_BASE + 0x004)

/* Pin interrupt level (rising edge) interrupt set register */
#define GPIO_PIN_INTERRUPT_SIENR        MMIO32(GPIO_PIN_INTERRUPT_BASE + 0x008)

/* Pin interrupt level (rising edge interrupt) clear register */
#define GPIO_PIN_INTERRUPT_CIENR        MMIO32(GPIO_PIN_INTERRUPT_BASE + 0x00C)

/* Pin interrupt active level (falling edge) interrupt enable register */
#define GPIO_PIN_INTERRUPT_IENF         MMIO32(GPIO_PIN_INTERRUPT_BASE + 0x010)

/* Pin interrupt active level (falling edge) interrupt set register */
#define GPIO_PIN_INTERRUPT_SIENF        MMIO32(GPIO_PIN_INTERRUPT_BASE + 0x014)

/* Pin interrupt active level (falling edge) interrupt clear register */
#define GPIO_PIN_INTERRUPT_CIENF        MMIO32(GPIO_PIN_INTERRUPT_BASE + 0x018)

/* Pin interrupt rising edge register */
#define GPIO_PIN_INTERRUPT_RISE         MMIO32(GPIO_PIN_INTERRUPT_BASE + 0x01C)

/* Pin interrupt falling edge register */
#define GPIO_PIN_INTERRUPT_FALL         MMIO32(GPIO_PIN_INTERRUPT_BASE + 0x020)

/* Pin interrupt status register */
#define GPIO_PIN_INTERRUPT_IST          MMIO32(GPIO_PIN_INTERRUPT_BASE + 0x024)




/**
 * Enumeration describing the types of GPIO interrupts we support.
 */
enum gpio_interrupt_sensitivity {
    LEVEL_SENSITIVE_HIGH   = 0,
    LEVEL_SENSITIVE_LOW    = 1,
    EDGE_SENSITIVE_RISING  = 2,
    EDGE_SENSITIVE_FALLING = 3,
    EDGE_SENSITIVE_BOTH    = 4
};
typedef enum gpio_interrupt_sensitivity gpio_interrupt_sensitivity_t;


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
    void *isr, int interrupt_priority);


/**
 * Enables a given GPIO interrupt previously configured e.g. with gpio_interrupt_configure.
 *
 * @param interrupt_number The GPIO INT#, from 0-7. This value has no relation to the GPIO pin,
 *    but instead indicates which of eight equivalent resources will be used.
 */
void gpio_interrupt_enable(int interrupt_number);


/**
 * Disables a given GPIO interrupt previously configured e.g. with gpio_interrupt_configure.
 *
 * @param interrupt_number The GPIO INT#, from 0-7. This value has no relation to the GPIO pin,
 *    but instead indicates which of eight equivalent resources will be used.
 */
void gpio_interrupt_disable(int interrupt_number);


#endif
