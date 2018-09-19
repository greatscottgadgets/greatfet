/*
 * This file is part of GreatFET
 */

#ifndef __USB_REGISTERS_H__
#define __USB_REGISTERS_H__

/**
 * Structure describing the USB register blocks.
 * Used so we don't have to have tons of constants floating around.
 */
typedef struct {
	uint32_t RESERVED0[64];
	uint32_t CAPLENGTH;
	uint32_t HCSPARAMS;
	uint32_t HCCPARAMS;
	uint32_t RESERVED1[5];
	uint32_t DCIVERSION;
	uint32_t RESERVED2[7];

	uint32_t USBCMD;
	uint32_t USBSTS;
	uint32_t USBINTR;
	uint32_t FRINDEX;

	uint32_t RESERVED3;

	union {
		 uint32_t PERIODICLISTBASE;
		 uint32_t DEVICEADDR;
	};

	union {
		 uint32_t ASYNCLISTADDR;
		 uint32_t ENDPOINTLISTADDR;
	};

	uint32_t TTCTRL;
	uint32_t BURSTSIZE;
	uint32_t TXFILLTUNING;
	uint32_t RESERVED4[2];
	uint32_t ULPIVIEWPORT;
	uint32_t BINTERVAL;
	uint32_t ENDPTNAK;
	uint32_t ENDPTNAKEN;
	uint32_t RESERVED5;
	uint32_t PORTSC1;
	uint32_t RESERVED6[7];
	uint32_t OTGSC;
	uint32_t USBMODE;
	uint32_t ENDPTSETUPSTAT;
	uint32_t ENDPTPRIME;
	uint32_t ENDPTFLUSH;
	uint32_t ENDPTSTAT;
	uint32_t ENDPTCOMPLETE;
	uint32_t ENDPTCTRL[6];

} usb_register_block_t;

/**
 * Quick references to the USB0 and USB1 register blocks.
 */

#define USB0_BLOCK		((volatile usb_register_block_t *)USB0_BASE)
#define USB1_BLOCK		((volatile usb_register_block_t *)USB1_BASE)

/**
 * Convenience macro for accessing the USB0/1 blocks programmatically.
 */
#define USB_REG(n)		((n) ? USB1_BLOCK : USB0_BLOCK)

#endif
