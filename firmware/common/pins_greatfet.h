/*
 * This file is part of GreatFET
 */

#ifndef __GREATFET_PINS_H
#define __GREATFET_PINS_H

#include <gpio_lpc.h>
#include <libopencm3/lpc43xx/scu.h>

#ifndef GREATFET_ONE
#error pins_greatfet.h included for a non-greatfet board! Use pins.h instead!
#endif

/* We populate the RTC crystal, so use of the RTC is allowed. */
#define BOARD_CAPABILITY_RTC

/* We can detect whether USB1's VBUS is present. */
#define BOARD_CAPABILITY_USB1_SENSE_VBUS
extern struct gpio_t gpio_usb1_sense;

/* We can provide VBUS to devices on USB1. */
#define BOARD_CAPABILITY_USB1_PROVIDE_VBUS
extern struct gpio_t gpio_usb1_en;


/*
 * SCU PinMux
 */

/* GPIO Output PinMux */

#define NUM_LEDS 4

#define SCU_PINMUX_LED1     (P7_6)  /* GPIO3[14] on P7_6 */
#define SCU_PINMUX_LED2     (P4_1)  /* GPIO2[1] on P4_1 */
#define SCU_PINMUX_LED3     (P7_5)  /* GPIO3[13] on P7_5 */
#define SCU_PINMUX_LED4     (P7_4)  /* GPIO3[12] on P7_4 */

#define PIN_LED1            (14) /* GPIO3[14] on P7_6 */
#define PIN_LED2            (1)  /* GPIO2[1] on P4_1 */
#define PIN_LED3            (13) /* GPIO3[13] on P7_5 */
#define PIN_LED4            (12) /* GPIO3[12] on P7_4 */

#define PORT_LED1           (3) /* PORT for LED1, 3, 4 */
#define PORT_LED2           (2) /* PORT for LED2 */
#define PORT_LED3           (3) /* PORT for LED1, 3, 4 */
#define PORT_LED4           (3) /* PORT for LED1, 3, 4 */

/* GPIO Output PinMux */
static const struct gpio_t gpio_led[NUM_LEDS] = {
	GPIO(PORT_LED1,  PIN_LED1),
	GPIO(PORT_LED2,  PIN_LED2),
	GPIO(PORT_LED3,  PIN_LED3),
	GPIO(PORT_LED4,  PIN_LED4)
};

static const scu_grp_pin_t pinmux_led[NUM_LEDS] = {
  SCU_PINMUX_LED1,
  SCU_PINMUX_LED2,
  SCU_PINMUX_LED3,
  SCU_PINMUX_LED4
};

static const scu_grp_pin_t scu_type_led[NUM_LEDS] = {
  SCU_GPIO_NOPULL | SCU_CONF_FUNCTION0,
  SCU_GPIO_NOPULL | SCU_CONF_FUNCTION0,
  SCU_GPIO_NOPULL | SCU_CONF_FUNCTION0,
  SCU_GPIO_NOPULL | SCU_CONF_FUNCTION0,
};


/* GPIO Input PinMux */
#define SCU_PINMUX_BOOT0    (P1_1)  /* GPIO0[8] on P1_1 */
#define SCU_PINMUX_BOOT1    (P1_2)  /* GPIO0[9] on P1_2 */
#define SCU_PINMUX_BOOT2    (P2_8)  /* GPIO5[7] on P2_8 */
#define SCU_PINMUX_BOOT3    (P2_9)  /* GPIO1[10] on P2_9 */

/* SSP1 Peripheral PinMux */
#define SCU_SSP1_MISO       (P1_3)  /* P1_3 */
#define SCU_SSP1_MOSI       (P1_4)  /* P1_4 */
#define SCU_SSP1_SCK        (P1_19) /* P1_19 */
#define SCU_SSP1_SSEL       (P1_20) /* P1_20 */

/* JTAG interface */
#define SCU_PINMUX_TDO (P9_5)  /* GPIO5[18] */
#define SCU_PINMUX_TCK (P6_1)  /* GPIO3[ 0] */
#define SCU_PINMUX_TMS (P6_5)  /* GPIO3[ 4] */
#define SCU_PINMUX_TDI (P6_2)  /* GPIO3[ 1] */


/* CPLD SGPIO interface */
#define SCU_PINMUX_SGPIO0   (P0_0)
#define SCU_PINMUX_SGPIO1   (P0_1)
#define SCU_PINMUX_SGPIO2   (P1_15)
#define SCU_PINMUX_SGPIO3   (P1_16)
#define SCU_PINMUX_SGPIO4   (P6_3)
#define SCU_PINMUX_SGPIO5   (P6_6)
#define SCU_PINMUX_SGPIO6   (P2_2)
#define SCU_PINMUX_SGPIO7   (P1_0)
#define SCU_PINMUX_SGPIO8   (P9_6)
#define SCU_PINMUX_SGPIO9   (P4_3)
#define SCU_PINMUX_SGPIO10  (P1_14)
#define SCU_PINMUX_SGPIO11  (P1_17)
#define SCU_PINMUX_SGPIO12  (P1_18)
#define SCU_PINMUX_SGPIO13  (P4_8)
#define SCU_PINMUX_SGPIO14  (P4_9)
#define SCU_PINMUX_SGPIO15  (P4_10)

/* SPI flash */
#define SCU_SSP0_MISO       (P3_6)
#define SCU_SSP0_MOSI       (P3_7)
#define SCU_SSP0_SCK        (P3_3)
#define SCU_SSP0_SSEL       (P3_8) /* GPIO5[11] on P3_8 */
#define SCU_FLASH_HOLD      (P3_4) /* GPIO1[14] on P3_4 */
#define SCU_FLASH_WP        (P3_5) /* GPIO1[15] on P3_5 */

#define ONBOARD_FLASH_DEVICE_ID  0x14 /* Expected device_id for W25Q16DV */
#define ONBOARD_FLASH_PAGE_LEN   256U
#define ONBOARD_FLASH_NUM_PAGES  8192U
#define ONBOARD_FLASH_NUM_BYTES  (ONBOARD_FLASH_PAGE_LEN * ONBOARD_FLASH_NUM_PAGES)


/* TODO add other Pins */
#define SCU_PINMUX_GPIO1_0  (P1_7)  /* GPIO1[0] */
#define SCU_PINMUX_GPIO1_1  (P1_8)  /* GPIO1[1] */
#define SCU_PINMUX_GPIO1_2  (P1_9)  /* GPIO1[2] */
#define SCU_PINMUX_GPIO1_3  (P1_10)  /* GPIO1[3] */
#define SCU_PINMUX_GPIO1_4  (P1_11)  /* GPIO1[4] */
#define SCU_PINMUX_GPIO1_5  (P1_12)  /* GPIO1[5] */
#define SCU_PINMUX_GPIO1_6  (P1_13)  /* GPIO1[6] */
#define SCU_PINMUX_GPIO1_7  (P1_14)  /* GPIO1[7] */

#define SCU_PINMUX_GPIO1_8  (P1_5)  /* GPIO1[8] */
#define SCU_PINMUX_GPIO1_9  (P1_6)  /* GPIO1[9] */
#define SCU_PINMUX_GPIO2_9  (P5_0)  /* GPIO2[9] */
#define SCU_PINMUX_GPIO2_11 (P5_2)  /* GPIO2[11] */
#define SCU_PINMUX_GPIO2_12 (P5_3)  /* GPIO2[12] */
#define SCU_PINMUX_GPIO2_14 (P5_5)  /* GPIO2[14] */
#define SCU_PINMUX_GPIO2_15 (P5_6)  /* GPIO2[15] */

#define SCU_PINMUX_GPIO3_8  (P7_0)  /* GPIO3[8] */
#define SCU_PINMUX_GPIO3_9  (P7_1)  /* GPIO3[9] */
#define SCU_PINMUX_GPIO3_10 (P7_2)  /* GPIO3[10] */
#define SCU_PINMUX_GPIO3_11 (P7_3)  /* GPIO3[11] */
#define SCU_PINMUX_GPIO3_12 (P7_4)  /* GPIO3[12] */
#define SCU_PINMUX_GPIO3_13 (P7_5)  /* GPIO3[13] */
#define SCU_PINMUX_GPIO3_14 (P7_6)  /* GPIO3[14] */
#define SCU_PINMUX_GPIO3_15 (P7_7)  /* GPIO3[15] */
#define SCU_PINMUX_GPIO5_3  (P2_3)  /* GPIO5[3] */
#define SCU_PINMUX_GPIO5_5  (P2_5)  /* GPIO5[5] */

#define SCU_PINMUX_GPIO1_8  (P1_5)  /* GPIO1[8] */

#define SCU_PINMUX_SD_POW   (P1_5)  /* GPIO1[8] */
#define SCU_PINMUX_SD_CMD   (P1_6)  /* GPIO1[9] */
#define SCU_PINMUX_SD_VOLT0 (P1_8)  /* GPIO1[1] */
#define SCU_PINMUX_SD_DAT0  (P1_9)  /* GPIO1[2] */
#define SCU_PINMUX_SD_DAT1  (P1_10) /* GPIO1[3] */
#define SCU_PINMUX_SD_DAT2  (P1_11) /* GPIO1[4] */
#define SCU_PINMUX_SD_DAT3  (P1_12) /* GPIO1[5] */
#define SCU_PINMUX_SD_CD    (P1_13) /* GPIO1[6] */

#define SCU_PINMUX_U0_TXD   (P2_0)  /* GPIO5[0] */
#define SCU_PINMUX_U0_RXD   (P2_1)  /* GPIO5[1] */

#define SCU_PINMUX_ISP      (P2_7)  /* GPIO0[7] */

#define SCU_PINMUX_GP_CLKIN	(P4_7)

#define SCU_PINMUX_USB1_SENSE    (P6_11) /* GPIO3[7] */
#define SCU_PINMUX_USB1_EN       (P6_12) /* GPIO2[8] */
#define SCU_PINMUX_USB1_FAULT    (P7_3)  /* GPIO3[11] */

#define SCU_PINMUX_USB1_EN_PORT        (2)
#define SCU_PINMUX_USB1_SENSE_PORT     (3)

#define SCU_PINMUX_USB1_EN_PIN         (8)
#define SCU_PINMUX_USB1_SENSE_PIN      (7)

/* Use the first LED (on the top) as our heartbeat led. */
#define HEARTBEAT_LED LED1


#endif /* __GREATFET_PINS_H */
