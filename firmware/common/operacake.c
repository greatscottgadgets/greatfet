/*
 * This file is part of GreatFET
 */
#include <libopencm3/lpc43xx/scu.h>
#include <pins.h>
#include <i2c_bus.h>
#include <greatfet_core.h>
#include "operacake.h"

#define OPERACAKE_DEFULAT_ADDRESS 0x18

#define OPERACAKE_PIN_OE(x)      (x<<7)
#define OPERACAKE_PIN_U2CTRL1(x) (x<<6)
#define OPERACAKE_PIN_U2CTRL0(x) (x<<5)
#define OPERACAKE_PIN_U3CTRL1(x) (x<<4)
#define OPERACAKE_PIN_U3CTRL0(x) (x<<3)
#define OPERACAKE_PIN_U1CTRL(x)  (x<<2)
#define OPERACAKE_PIN_LEDEN2(x)  (x<<1)
#define OPERACAKE_PIN_LEDEN(x)   (x<<0)

#define OPERACAKE_PORT_A1 (OPERACAKE_PIN_U2CTRL0(0) | OPERACAKE_PIN_U2CTRL1(0))
#define OPERACAKE_PORT_A2 (OPERACAKE_PIN_U2CTRL0(1) | OPERACAKE_PIN_U2CTRL1(0))
#define OPERACAKE_PORT_A3 (OPERACAKE_PIN_U2CTRL0(0) | OPERACAKE_PIN_U2CTRL1(1))
#define OPERACAKE_PORT_A4 (OPERACAKE_PIN_U2CTRL0(1) | OPERACAKE_PIN_U2CTRL1(1))

#define OPERACAKE_PORT_B1 (OPERACAKE_PIN_U3CTRL0(0) | OPERACAKE_PIN_U3CTRL1(0))
#define OPERACAKE_PORT_B2 (OPERACAKE_PIN_U3CTRL0(1) | OPERACAKE_PIN_U3CTRL1(0))
#define OPERACAKE_PORT_B3 (OPERACAKE_PIN_U3CTRL0(0) | OPERACAKE_PIN_U3CTRL1(1))
#define OPERACAKE_PORT_B4 (OPERACAKE_PIN_U3CTRL0(1) | OPERACAKE_PIN_U3CTRL1(1))

#define OPERACAKE_SAMESIDE  OPERACAKE_PIN_U1CTRL(1)
#define OPERACAKE_CROSSOVER OPERACAKE_PIN_U1CTRL(0)
#define OPERACAKE_EN_LEDS (OPERACAKE_PIN_LEDEN2(1) | OPERACAKE_PIN_LEDEN2(0))
#define OPERACAKE_GPIO_EN OPERACAKE_PIN_OE(0)
#define OPERACAKE_GPIO_DISABLE OPERACAKE_PIN_OE(1)

#define OPERACAKE_REG_INPUT    0x00
#define OPERACAKE_REG_OUTPUT   0x01
#define OPERACAKE_REG_POLARITY 0x02
#define OPERACAKE_REG_CONFIG   0x03

// OE and LEDs are outputs, the rest are not
#define OPERACAKE_CONFIG_OUTPUTS (0x7C)

#define OPERACAKE_DEFAULT_OUTPUT (OPERACAKE_GPIO_DISABLE | OPERACAKE_SAMESIDE \
								  | OPERACAKE_PORT_A1 | OPERACAKE_PORT_B1 \
								  | OPERACAKE_EN_LEDS)

static struct gpio_t u1ctrl  = GPIO(2, 5);
static struct gpio_t u2ctrl0 = GPIO(2, 3);
static struct gpio_t u2ctrl1 = GPIO(2, 6);
static struct gpio_t u3ctrl0 = GPIO(2, 4);
static struct gpio_t u3ctrl1 = GPIO(2, 2);

/* read single register */
uint8_t operacake_read_reg(i2c_bus_t* const bus, uint8_t reg) {
	const uint8_t data_tx[] = { reg };
	uint8_t data_rx[] = { 0x00 };
	i2c_bus_transfer(bus, OPERACAKE_DEFULAT_ADDRESS, data_tx, 1, data_rx, 1);
	return data_rx[0];
}

/* Write to one of the PCA9557 registers */
void operacake_write_reg(i2c_bus_t* const bus, uint8_t reg, uint8_t value) {
	const uint8_t data[] = {reg, value};
	i2c_bus_transfer(bus, OPERACAKE_DEFULAT_ADDRESS, data, 2, NULL, 0);
}

uint8_t operacake_init(void) {
    uint8_t reg;
    // U1CTRL
    scu_pinmux(SCU_PINMUX_GPIO2_5, SCU_CONF_EPUN_DIS_PULLUP | SCU_CONF_EHS_FAST | SCU_CONF_FUNCTION1);
    gpio_output(&u1ctrl);
    gpio_write(&u1ctrl, 1);
    // U2CTRL0
    scu_pinmux(SCU_PINMUX_GPIO2_3, SCU_CONF_EPUN_DIS_PULLUP | SCU_CONF_EHS_FAST | SCU_CONF_FUNCTION1);
    gpio_output(&u2ctrl0);
    gpio_write(&u2ctrl0, 0);
    // U2CTRL1
    scu_pinmux(SCU_PINMUX_GPIO2_6, SCU_CONF_EPUN_DIS_PULLUP | SCU_CONF_EHS_FAST | SCU_CONF_FUNCTION1);
    gpio_output(&u2ctrl1);
    gpio_write(&u2ctrl1, 0);
    // U3CTRL0
    scu_pinmux(SCU_PINMUX_GPIO2_4, SCU_CONF_EPUN_DIS_PULLUP | SCU_CONF_EHS_FAST | SCU_CONF_FUNCTION1);
    gpio_output(&u3ctrl0);
    gpio_write(&u3ctrl0, 0);
    // U3CTRL1
    scu_pinmux(SCU_PINMUX_GPIO2_2, SCU_CONF_EPUN_DIS_PULLUP | SCU_CONF_EHS_FAST | SCU_CONF_FUNCTION1);
    gpio_output(&u3ctrl1);
    gpio_write(&u3ctrl1, 0);

	i2c_bus_start(&i2c0, &i2c_config_fast_clock);
	operacake_write_reg(&i2c0, OPERACAKE_REG_OUTPUT, OPERACAKE_DEFAULT_OUTPUT);
	operacake_write_reg(&i2c0, OPERACAKE_REG_CONFIG, OPERACAKE_CONFIG_OUTPUTS);
	reg = operacake_read_reg(&i2c0, OPERACAKE_REG_CONFIG);
	i2c_bus_stop(&i2c0);
    if(reg==OPERACAKE_CONFIG_OUTPUTS)
        return 0;
    return 1;
 }
 