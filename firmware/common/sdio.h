/*
 * This file is part of GreatFET
 */

#ifndef LPC43XX_SDIO_H
#define LPC43XX_SDIO_H

/**@{*/

#include <libopencm3/cm3/common.h>
#include <libopencm3/lpc43xx/memorymap.h>

#ifdef __cplusplus
extern "C" {
#endif

/* --- SDIO registers ----------------------------------------------------- */

/* Control Register */
#define SDIO_CTRL                       MMIO32(SDIO_BASE + 0x000)

/* Power Enable Register */
#define SDIO_PWREN                      MMIO32(SDIO_BASE + 0x004)

/* Clock Divider Register */
#define SDIO_CLKDIV                     MMIO32(SDIO_BASE + 0x008)

/* SD Clock Source Register */
#define SDIO_CLKSRC                     MMIO32(SDIO_BASE + 0x00C)

/* Clock Enable Register */
#define SDIO_CLKENA                     MMIO32(SDIO_BASE + 0x010)

/* Time-out Register */
#define SDIO_TMOUT                      MMIO32(SDIO_BASE + 0x014)

/* Card Type Register */
#define SDIO_CTYPE                      MMIO32(SDIO_BASE + 0x018)

/* Block Size Register */
#define SDIO_BLKSIZ                     MMIO32(SDIO_BASE + 0x01C)

/* Byte Count Register */
#define SDIO_BYTCNT                     MMIO32(SDIO_BASE + 0x020)

/* Interrupt Mask Register */
#define SDIO_INTMASK                    MMIO32(SDIO_BASE + 0x024)

/* Command Argument Register */
#define SDIO_CMDARG                     MMIO32(SDIO_BASE + 0x028)

/* Command Register */
#define SDIO_CMD                        MMIO32(SDIO_BASE + 0x02C)

/* Response Register 0 */
#define SDIO_RESP0                      MMIO32(SDIO_BASE + 0x030)

/* Response Register 1 */
#define SDIO_RESP1                      MMIO32(SDIO_BASE + 0x034)

/* Response Register 2 */
#define SDIO_RESP2                      MMIO32(SDIO_BASE + 0x038)

/* Response Register 3 */
#define SDIO_RESP3                      MMIO32(SDIO_BASE + 0x03C)

/* Masked Interrupt Status Register */
#define SDIO_MINTSTS                    MMIO32(SDIO_BASE + 0x040)

/* Raw Interrupt Status Register */
#define SDIO_RINTSTS                    MMIO32(SDIO_BASE + 0x044)

/* Status Register */
#define SDIO_STATUS                     MMIO32(SDIO_BASE + 0x048)

/* FIFO Threshold Watermark Register */
#define SDIO_FIFOTH                     MMIO32(SDIO_BASE + 0x04C)

/* Card Detect Register */
#define SDIO_CDETECT                    MMIO32(SDIO_BASE + 0x050)

/* Write Protect Register */
#define SDIO_WRTPRT                     MMIO32(SDIO_BASE + 0x054)

/* Transferred CIU Card Byte Count Register */
#define SDIO_TCBCNT                     MMIO32(SDIO_BASE + 0x05C)

/* Transferred Host to BIU-FIFO Byte Count Register */
#define SDIO_TBBCNT                     MMIO32(SDIO_BASE + 0x060)

/* Debounce Count Register */
#define SDIO_DEBNCE                     MMIO32(SDIO_BASE + 0x064)

/* Hardware Reset */
#define SDIO_RST_N                      MMIO32(SDIO_BASE + 0x078)

/* Bus Mode Register */
#define SDIO_BMOD                       MMIO32(SDIO_BASE + 0x080)

/* Poll Demand Register */
#define SDIO_PLDMND                     MMIO32(SDIO_BASE + 0x084)

/* Descriptor List Base Address Register */
#define SDIO_DBADDR                     MMIO32(SDIO_BASE + 0x088)

/* Internal DMAC Status Register */
#define SDIO_IDSTS                      MMIO32(SDIO_BASE + 0x08C)

/* Internal DMAC Interrupt Enable Register */
#define SDIO_IDINTEN                    MMIO32(SDIO_BASE + 0x090)

/* Current Host Descriptor Address Register */
#define SDIO_DSCADDR                    MMIO32(SDIO_BASE + 0x094)

/* Current Buffer Descriptor Address Register */
#define SDIO_BUFADDR                    MMIO32(SDIO_BASE + 0x098)

/* Data FIFO read/write */
#define SDIO_DATA                       MMIO32(SDIO_BASE (1)

/* --- SDIO_CTRL values ----------------------------------------- */

/* CONTROLLER_RESET: Controller reset */
#define SDIO_CTRL_CONTROLLER_RESET_SHIFT (0)
#define SDIO_CTRL_CONTROLLER_RESET (1 << SDIO_CTRL_CONTROLLER_RESET_SHIFT)

/* FIFO_RESET: FIFO reset */
#define SDIO_CTRL_FIFO_RESET_SHIFT (1)
#define SDIO_CTRL_FIFO_RESET (1 << SDIO_CTRL_FIFO_RESET_SHIFT)

/* DMA_RESET: DMA reset */
#define SDIO_CTRL_DMA_RESET_SHIFT (2)
#define SDIO_CTRL_DMA_RESET (1 << SDIO_CTRL_DMA_RESET_SHIFT)

/* INT_ENABLE: Global interrupt enable/disable */
#define SDIO_CTRL_INT_ENABLE_SHIFT (4)
#define SDIO_CTRL_INT_ENABLE (1 << SDIO_CTRL_INT_ENABLE_SHIFT)

/* READ_WAIT: Read/wait send */
#define SDIO_CTRL_READ_WAIT_SHIFT (6)
#define SDIO_CTRL_READ_WAIT (1 << SDIO_CTRL_READ_WAIT_SHIFT)

/* SEND_IRQ_RESPONSE: Send IRQ response */
#define SDIO_CTRL_SEND_IRQ_RESPONSE_SHIFT (7)
#define SDIO_CTRL_SEND_IRQ_RESPONSE (1 << SDIO_CTRL_SEND_IRQ_RESPONSE_SHIFT)

/* ABORT_READ_DATA: Abort read data */
#define SDIO_CTRL_ABORT_READ_DATA_SHIFT (8)
#define SDIO_CTRL_ABORT_READ_DATA (1DIO_CTRL_ABORT_READ_DATA_SHIFT)

/* SEND_CCSD: Send CCSD */
#define SDIO_CTRL_SEND_CCSD_SHIFT (9)
#define SDIO_CTRL_SEND_CCSD (1DIO_CTRL_SEND_CCSD_SHIFT)

/* SEND_AUTO_STOP_CCSD: Send auto stop CCSD */
#define SDIO_CTRL_SEND_AUTO_STOP_CCSD_SHIFT (10)
#define SDIO_CTRL_SEND_AUTO_STOP_CCSD (1DIO_CTRL_SEND_AUTO_STOP_CCSD_SHIFT)

/* CEATA_DEVICE_INTERRUPT_STATUS: CEATA device interrupt status */
#define SDIO_CTRL_CEATA_DEVICE_INTERRUPT_STATUS_SHIFT (11)
#define SDIO_CTRL_CEATA_DEVICE_INTERRUPT_STATUS (1DIO_CTRL_CEATA_DEVICE_INTERRUPT_STATUS_SHIFT)

/* CARD_VOLTAGE_A0: SD_VOLT0 pin control */
#define SDIO_CTRL_CARD_VOLTAGE_A0_SHIFT (16)
#define SDIO_CTRL_CARD_VOLTAGE_A0 (1DIO_CTRL_CARD_VOLTAGE_A0_SHIFT)

/* CARD_VOLTAGE_A1: SD_VOLT1 pin control */
#define SDIO_CTRL_CARD_VOLTAGE_A1_SHIFT (17)
#define SDIO_CTRL_CARD_VOLTAGE_A1 (1DIO_CTRL_CARD_VOLTAGE_A1_SHIFT)

/* CARD_VOLTAGE_A2: SD_VOLT2 pin control */
#define SDIO_CTRL_CARD_VOLTAGE_A2_SHIFT (18)
#define SDIO_CTRL_CARD_VOLTAGE_A2 (1DIO_CTRL_CARD_VOLTAGE_A2_SHIFT)

/* USE_INTERNAL_DMAC: SD/MMC DMA use */
#define SDIO_CTRL_USE_INTERNAL_DMAC_SHIFT (25)
#define SDIO_CTRL_USE_INTERNAL_DMAC (1DIO_CTRL_USE_INTERNAL_DMAC_SHIFT)

/* --- SDIO_PWREN values ---------------------------------------- */

/* POWER_ENABLE: Power on/off switch for card */
#define SDIO_PWREN_POWER_ENABLE_SHIFT (0)
#define SDIO_PWREN_POWER_ENABLE (1DIO_PWREN_POWER_ENABLE_SHIFT)

/* --- SDIO_CLKDIV values --------------------------------------- */

/* CLK_DIVIDER0: Clock divider-0 value */
#define SDIO_CLKDIV_CLK_DIVIDER0_SHIFT (0)
#define SDIO_CLKDIV_CLK_DIVIDER0_MASK (0xff << SDIO_CLKDIV_CLK_DIVIDER0_SHIFT)
#define SDIO_CLKDIV_CLK_DIVIDER0(x) ((x) << SDIO_CLKDIV_CLK_DIVIDER0_SHIFT)

/* CLK_DIVIDER1: Clock divider-1 value */
#define SDIO_CLKDIV_CLK_DIVIDER1_SHIFT (8)
#define SDIO_CLKDIV_CLK_DIVIDER1_MASK (0xff << SDIO_CLKDIV_CLK_DIVIDER1_SHIFT)
#define SDIO_CLKDIV_CLK_DIVIDER1(x) ((x) << SDIO_CLKDIV_CLK_DIVIDER1_SHIFT)

/* CLK_DIVIDER2: Clock divider-2 value */
#define SDIO_CLKDIV_CLK_DIVIDER2_SHIFT (16)
#define SDIO_CLKDIV_CLK_DIVIDER2_MASK (0xff << SDIO_CLKDIV_CLK_DIVIDER2_SHIFT)
#define SDIO_CLKDIV_CLK_DIVIDER2(x) ((x) << SDIO_CLKDIV_CLK_DIVIDER2_SHIFT)

/* CLK_DIVIDER3: Clock divider-3 value */
#define SDIO_CLKDIV_CLK_DIVIDER3_SHIFT (24)
#define SDIO_CLKDIV_CLK_DIVIDER3_MASK (0xff << SDIO_CLKDIV_CLK_DIVIDER3_SHIFT)
#define SDIO_CLKDIV_CLK_DIVIDER3(x) ((x) << SDIO_CLKDIV_CLK_DIVIDER3_SHIFT)

/* --- SDIO_CLKSRC values --------------------------------------- */

/* CLK_SOURCE: Clock divider source for SD card */
#define SDIO_CLKSRC_CLK_SOURCE_SHIFT (0)
#define SDIO_CLKSRC_CLK_SOURCE_MASK (0x3 << SDIO_CLKSRC_CLK_SOURCE_SHIFT)
#define SDIO_CLKSRC_CLK_SOURCE(x) ((x) << SDIO_CLKSRC_CLK_SOURCE_SHIFT)

/* --- SDIO_CLKENA values --------------------------------------- */

/* CCLK_ENABLE: Clock-enable control for SD card clock */
#define SDIO_CLKENA_CCLK_ENABLE_SHIFT (0)
#define SDIO_CLKENA_CCLK_ENABLE (1DIO_CLKENA_CCLK_ENABLE_SHIFT)

/* CCLK_LOW_POWER: Low-power control for SD card clock */
#define SDIO_CLKENA_CCLK_LOW_POWER_SHIFT (16)
#define SDIO_CLKENA_CCLK_LOW_POWER (1 << SDIO_CLKENA_CCLK_LOW_POWER_SHIFT)

/* --- SDIO_TMOUT values ---------------------------------------- */

/* RESPONSE_TIMEOUT: Response time-out value */
#define SDIO_TMOUT_RESPONSE_TIMEOUT_SHIFT (0)
#define SDIO_TMOUT_RESPONSE_TIMEOUT_MASK (0xff << SDIO_TMOUT_RESPONSE_TIMEOUT_SHIFT)
#define SDIO_TMOUT_RESPONSE_TIMEOUT(x) ((x) << SDIO_TMOUT_RESPONSE_TIMEOUT_SHIFT)

/* DATA_TIMEOUT: Value for card data read time-out */
#define SDIO_TMOUT_DATA_TIMEOUT_SHIFT (8)
#define SDIO_TMOUT_DATA_TIMEOUT_MASK (0xffffff << SDIO_TMOUT_DATA_TIMEOUT_SHIFT)
#define SDIO_TMOUT_DATA_TIMEOUT(x) ((x) << SDIO_TMOUT_DATA_TIMEOUT_SHIFT)

/* --- SDIO_CTYPE values ---------------------------------------- */

/* CARD_WIDTH0: Indicates if card is 1-bit or 4-bit */
#define SDIO_CTYPE_CARD_WIDTH0_SHIFT (0)
#define SDIO_CTYPE_CARD_WIDTH0 (1 << SDIO_CTYPE_CARD_WIDTH0_SHIFT)

/* CARD_WIDTH1: Indicates if card is 8-bit */
#define SDIO_CTYPE_CARD_WIDTH1_SHIFT (16)
#define SDIO_CTYPE_CARD_WIDTH1 (1 << SDIO_CTYPE_CARD_WIDTH1_SHIFT)

/* --- SDIO_BLKSIZ values --------------------------------------- */

/* BLOCK_SIZE: Block size */
#define SDIO_BLKSIZ_BLOCK_SIZE_SHIFT (0)
#define SDIO_BLKSIZ_BLOCK_SIZE_MASK (0xffff << SDIO_BLKSIZ_BLOCK_SIZE_SHIFT)
#define SDIO_BLKSIZ_BLOCK_SIZE(x) ((x) << SDIO_BLKSIZ_BLOCK_SIZE_SHIFT)

/* --- SDIO_BYTCNT values --------------------------------------- */

/* BYTE_COUNT: Number of bytes to be transferred */
#define SDIO_BYTCNT_BYTE_COUNT_SHIFT (0)
#define SDIO_BYTCNT_BYTE_COUNT_MASK (0xffffffff << SDIO_BYTCNT_BYTE_COUNT_SHIFT)
#define SDIO_BYTCNT_BYTE_COUNT(x) ((x) << SDIO_BYTCNT_BYTE_COUNT_SHIFT)

/* --- SDIO_INTMASK values -------------------------------------- */

/* CDET: Card detect */
#define SDIO_INTMASK_CDET_SHIFT (0)
#define SDIO_INTMASK_CDET (1 << SDIO_INTMASK_CDET_SHIFT)

/* RE: Response error */
#define SDIO_INTMASK_RE_SHIFT (1)
#define SDIO_INTMASK_RE (1 << SDIO_INTMASK_RE_SHIFT)

/* CDONE: Command done */
#define SDIO_INTMASK_CDONE_SHIFT (2)
#define SDIO_INTMASK_CDONE (1 << SDIO_INTMASK_CDONE_SHIFT)

/* DTO: Data transfer over */
#define SDIO_INTMASK_DTO_SHIFT (3)
#define SDIO_INTMASK_DTO (1 << SDIO_INTMASK_DTO_SHIFT)

/* TXDR: Transmit FIFO data request */
#define SDIO_INTMASK_TXDR_SHIFT (4)
#define SDIO_INTMASK_TXDR (1 << SDIO_INTMASK_TXDR_SHIFT)

/* RXDR: Receive FIFO data request */
#define SDIO_INTMASK_RXDR_SHIFT (5)
#define SDIO_INTMASK_RXDR (1 << SDIO_INTMASK_RXDR_SHIFT)

/* RCRC: Response CRC error */
#define SDIO_INTMASK_RCRC_SHIFT (6)
#define SDIO_INTMASK_RCRC (1 << SDIO_INTMASK_RCRC_SHIFT)

/* DCRC: Data CRC error */
#define SDIO_INTMASK_DCRC_SHIFT (7)
#define SDIO_INTMASK_DCRC (1 << SDIO_INTMASK_DCRC_SHIFT)

/* RTO: Response time-out */
#define SDIO_INTMASK_RTO_SHIFT (8)
#define SDIO_INTMASK_RTO (1 << SDIO_INTMASK_RTO_SHIFT)

/* DRTO: Data read time-out */
#define SDIO_INTMASK_DRTO_SHIFT (9)
#define SDIO_INTMASK_DRTO (1 << SDIO_INTMASK_DRTO_SHIFT)

/* HTO: Data starvation-by-host time-out/volt_switch_int */
#define SDIO_INTMASK_HTO_SHIFT (10)
#define SDIO_INTMASK_HTO (1 << SDIO_INTMASK_HTO_SHIFT)

/* FRUN: FIFO underrun/overrun error */
#define SDIO_INTMASK_FRUN_SHIFT (11)
#define SDIO_INTMASK_FRUN (1 << SDIO_INTMASK_FRUN_SHIFT)

/* HLE: Hardware locked write error */
#define SDIO_INTMASK_HLE_SHIFT (12)
#define SDIO_INTMASK_HLE (1 << SDIO_INTMASK_HLE_SHIFT)

/* SBE: Start-bit error */
#define SDIO_INTMASK_SBE_SHIFT (13)
#define SDIO_INTMASK_SBE (1 << SDIO_INTMASK_SBE_SHIFT)

/* ACD: Auto command done */
#define SDIO_INTMASK_ACD_SHIFT (14)
#define SDIO_INTMASK_ACD (1 << SDIO_INTMASK_ACD_SHIFT)

/* EBE: End-bit error (read)/Write no CRC */
#define SDIO_INTMASK_EBE_SHIFT (15)
#define SDIO_INTMASK_EBE (1 << SDIO_INTMASK_EBE_SHIFT)

/* SDIO_INT_MASK: Mask SDIO interrupt */
#define SDIO_INTMASK_SDIO_INT_MASK_SHIFT (16)
#define SDIO_INTMASK_SDIO_INT_MASK (1 << SDIO_INTMASK_SDIO_INT_MASK_SHIFT)

/* --- SDIO_CMDARG values --------------------------------------- */

/* CMD_ARG: Value indicates command argument to be passed to card */
#define SDIO_CMDARG_CMD_ARG_SHIFT (0)
#define SDIO_CMDARG_CMD_ARG_MASK (0xffffffff << SDIO_CMDARG_CMD_ARG_SHIFT)
#define SDIO_CMDARG_CMD_ARG(x) ((x) << SDIO_CMDARG_CMD_ARG_SHIFT)

/* --- SDIO_CMD values ------------------------------------------ */

/* CMD_INDEX: Command index */
#define SDIO_CMD_CMD_INDEX_SHIFT (0)
#define SDIO_CMD_CMD_INDEX_MASK (0x3f << SDIO_CMD_CMD_INDEX_SHIFT)
#define SDIO_CMD_CMD_INDEX(x) ((x) << SDIO_CMD_CMD_INDEX_SHIFT)

/* RESPONSE_EXPECT: Response expect */
#define SDIO_CMD_RESPONSE_EXPECT_SHIFT (6)
#define SDIO_CMD_RESPONSE_EXPECT (1 << SDIO_CMD_RESPONSE_EXPECT_SHIFT)

/* RESPONSE_LENGTH: Response length */
#define SDIO_CMD_RESPONSE_LENGTH_SHIFT (7)
#define SDIO_CMD_RESPONSE_LENGTH (1 << SDIO_CMD_RESPONSE_LENGTH_SHIFT)

/* CHECK_RESPONSE_CRC: Check response CRC */
#define SDIO_CMD_CHECK_RESPONSE_CRC_SHIFT (8)
#define SDIO_CMD_CHECK_RESPONSE_CRC (1 << SDIO_CMD_CHECK_RESPONSE_CRC_SHIFT)

/* DATA_EXPECTED: Data expected */
#define SDIO_CMD_DATA_EXPECTED_SHIFT (9)
#define SDIO_CMD_DATA_EXPECTED (1 << SDIO_CMD_DATA_EXPECTED_SHIFT)

/* READ_WRITE: Read/write */
#define SDIO_CMD_READ_WRITE_SHIFT (10)
#define SDIO_CMD_READ_WRITE (1 << SDIO_CMD_READ_WRITE_SHIFT)

/* TRANSFER_MODE: Transfer mode */
#define SDIO_CMD_TRANSFER_MODE_SHIFT (11)
#define SDIO_CMD_TRANSFER_MODE (1 << SDIO_CMD_TRANSFER_MODE_SHIFT)

/* SEND_AUTO_STOP: Send auto stop */
#define SDIO_CMD_SEND_AUTO_STOP_SHIFT (12)
#define SDIO_CMD_SEND_AUTO_STOP (1 << SDIO_CMD_SEND_AUTO_STOP_SHIFT)

/* WAIT_PRVDATA_COMPLETE: Wait prvdata complete */
#define SDIO_CMD_WAIT_PRVDATA_COMPLETE_SHIFT (13)
#define SDIO_CMD_WAIT_PRVDATA_COMPLETE (1 << SDIO_CMD_WAIT_PRVDATA_COMPLETE_SHIFT)

/* STOP_ABORT_CMD: Stop abort command */
#define SDIO_CMD_STOP_ABORT_CMD_SHIFT (14)
#define SDIO_CMD_STOP_ABORT_CMD (1 << SDIO_CMD_STOP_ABORT_CMD_SHIFT)

/* SEND_INITIALIZATION: Send initialization */
#define SDIO_CMD_SEND_INITIALIZATION_SHIFT (15)
#define SDIO_CMD_SEND_INITIALIZATION (1 << SDIO_CMD_SEND_INITIALIZATION_SHIFT)

/* UPDATE_CLOCK_REGISTERS_ONLY: Update clock registers only */
#define SDIO_CMD_UPDATE_CLOCK_REGISTERS_ONLY_SHIFT (21)
#define SDIO_CMD_UPDATE_CLOCK_REGISTERS_ONLY (1 << SDIO_CMD_UPDATE_CLOCK_REGISTERS_ONLY_SHIFT)

/* READ_CEATA_DEVICE: Read CEATA device */
#define SDIO_CMD_READ_CEATA_DEVICE_SHIFT (22)
#define SDIO_CMD_READ_CEATA_DEVICE (1 << SDIO_CMD_READ_CEATA_DEVICE_SHIFT)

/* CCS_EXPECTED: CCS expected */
#define SDIO_CMD_CCS_EXPECTED_SHIFT (23)
#define SDIO_CMD_CCS_EXPECTED (1 << SDIO_CMD_CCS_EXPECTED_SHIFT)

/* ENABLE_BOOT: Enable boot */
#define SDIO_CMD_ENABLE_BOOT_SHIFT (24)
#define SDIO_CMD_ENABLE_BOOT (1 << SDIO_CMD_ENABLE_BOOT_SHIFT)

/* EXPECT_BOOT_ACK: Expect boot acknowledge */
#define SDIO_CMD_EXPECT_BOOT_ACK_SHIFT (25)
#define SDIO_CMD_EXPECT_BOOT_ACK (1 << SDIO_CMD_EXPECT_BOOT_ACK_SHIFT)

/* DISABLE_BOOT: Disable boot */
#define SDIO_CMD_DISABLE_BOOT_SHIFT (26)
#define SDIO_CMD_DISABLE_BOOT (1 << SDIO_CMD_DISABLE_BOOT_SHIFT)

/* BOOT_MODE: Boot mode */
#define SDIO_CMD_BOOT_MODE_SHIFT (27)
#define SDIO_CMD_BOOT_MODE (1 << SDIO_CMD_BOOT_MODE_SHIFT)

/* VOLT_SWITCH: Voltage switch bit */
#define SDIO_CMD_VOLT_SWITCH_SHIFT (28)
#define SDIO_CMD_VOLT_SWITCH (1 << SDIO_CMD_VOLT_SWITCH_SHIFT)

/* START_CMD: Start command */
#define SDIO_CMD_START_CMD_SHIFT (31)
#define SDIO_CMD_START_CMD (1 << SDIO_CMD_START_CMD_SHIFT)

/* --- SDIO_RESP0 values ---------------------------------------- */

/* RESPONSE0: Bit[31:0] of response */
#define SDIO_RESP0_RESPONSE0_SHIFT (0)
#define SDIO_RESP0_RESPONSE0_MASK (0xffffffff << SDIO_RESP0_RESPONSE0_SHIFT)
#define SDIO_RESP0_RESPONSE0(x) ((x) << SDIO_RESP0_RESPONSE0_SHIFT)

/* --- SDIO_RESP1 values ---------------------------------------- */

/* RESPONSE1: Bit[63:32] of long response */
#define SDIO_RESP1_RESPONSE1_SHIFT (0)
#define SDIO_RESP1_RESPONSE1_MASK (0xffffffff << SDIO_RESP1_RESPONSE1_SHIFT)
#define SDIO_RESP1_RESPONSE1(x) ((x) << SDIO_RESP1_RESPONSE1_SHIFT)

/* --- SDIO_RESP2 values ---------------------------------------- */

/* RESPONSE2: Bit[95:64] of long response */
#define SDIO_RESP2_RESPONSE2_SHIFT (0)
#define SDIO_RESP2_RESPONSE2_MASK (0xffffffff << SDIO_RESP2_RESPONSE2_SHIFT)
#define SDIO_RESP2_RESPONSE2(x) ((x) << SDIO_RESP2_RESPONSE2_SHIFT)

/* --- SDIO_RESP3 values ---------------------------------------- */

/* RESPONSE3: Bit[127:96] of long response */
#define SDIO_RESP3_RESPONSE3_SHIFT (0)
#define SDIO_RESP3_RESPONSE3_MASK (0xffffffff << SDIO_RESP3_RESPONSE3_SHIFT)
#define SDIO_RESP3_RESPONSE3(x) ((x) << SDIO_RESP3_RESPONSE3_SHIFT)

/* --- SDIO_MINTSTS values -------------------------------------- */

/* CDET: Card detect */
#define SDIO_MINTSTS_CDET_SHIFT (0)
#define SDIO_MINTSTS_CDET (1 << SDIO_MINTSTS_CDET_SHIFT)

/* RE: Response error */
#define SDIO_MINTSTS_RE_SHIFT (1)
#define SDIO_MINTSTS_RE (1 << SDIO_MINTSTS_RE_SHIFT)

/* CDONE: Command done */
#define SDIO_MINTSTS_CDONE_SHIFT (2)
#define SDIO_MINTSTS_CDONE (1 << SDIO_MINTSTS_CDONE_SHIFT)

/* DTO: Data transfer over */
#define SDIO_MINTSTS_DTO_SHIFT (3)
#define SDIO_MINTSTS_DTO (1 << SDIO_MINTSTS_DTO_SHIFT)

/* TXDR: Transmit FIFO data request */
#define SDIO_MINTSTS_TXDR_SHIFT (4)
#define SDIO_MINTSTS_TXDR (1 << SDIO_MINTSTS_TXDR_SHIFT)

/* RXDR: Receive FIFO data request */
#define SDIO_MINTSTS_RXDR_SHIFT (5)
#define SDIO_MINTSTS_RXDR (1 << SDIO_MINTSTS_RXDR_SHIFT)

/* RCRC: Response CRC error */
#define SDIO_MINTSTS_RCRC_SHIFT (6)
#define SDIO_MINTSTS_RCRC (1 << SDIO_MINTSTS_RCRC_SHIFT)

/* DCRC: Data CRC error */
#define SDIO_MINTSTS_DCRC_SHIFT (7)
#define SDIO_MINTSTS_DCRC (1 << SDIO_MINTSTS_DCRC_SHIFT)

/* RTO: Response time-out */
#define SDIO_MINTSTS_RTO_SHIFT (8)
#define SDIO_MINTSTS_RTO (1 << SDIO_MINTSTS_RTO_SHIFT)

/* DRTO: Data read time-out */
#define SDIO_MINTSTS_DRTO_SHIFT (9)
#define SDIO_MINTSTS_DRTO (1 << SDIO_MINTSTS_DRTO_SHIFT)

/* HTO: Data starvation-by-host time-out */
#define SDIO_MINTSTS_HTO_SHIFT (10)
#define SDIO_MINTSTS_HTO (1 << SDIO_MINTSTS_HTO_SHIFT)

/* FRUN: FIFO underrun/overrun error */
#define SDIO_MINTSTS_FRUN_SHIFT (11)
#define SDIO_MINTSTS_FRUN (1 << SDIO_MINTSTS_FRUN_SHIFT)

/* HLE: Hardware locked write error */
#define SDIO_MINTSTS_HLE_SHIFT (12)
#define SDIO_MINTSTS_HLE (1 << SDIO_MINTSTS_HLE_SHIFT)

/* SBE: Start-bit error */
#define SDIO_MINTSTS_SBE_SHIFT (13)
#define SDIO_MINTSTS_SBE (1 << SDIO_MINTSTS_SBE_SHIFT)

/* ACD: Auto command done */
#define SDIO_MINTSTS_ACD_SHIFT (14)
#define SDIO_MINTSTS_ACD (1 << SDIO_MINTSTS_ACD_SHIFT)

/* EBE: End-bit error (read)/write no CRC */
#define SDIO_MINTSTS_EBE_SHIFT (15)
#define SDIO_MINTSTS_EBE (1 << SDIO_MINTSTS_EBE_SHIFT)

/* SDIO_INTERRUPT: Interrupt from SDIO card */
#define SDIO_MINTSTS_SDIO_INTERRUPT_SHIFT (16)
#define SDIO_MINTSTS_SDIO_INTERRUPT (1 << SDIO_MINTSTS_SDIO_INTERRUPT_SHIFT)

/* --- SDIO_RINTSTS values -------------------------------------- */

/* CDET: Card detect */
#define SDIO_RINTSTS_CDET_SHIFT (0)
#define SDIO_RINTSTS_CDET (1 << SDIO_RINTSTS_CDET_SHIFT)

/* RE: Response error */
#define SDIO_RINTSTS_RE_SHIFT (1)
#define SDIO_RINTSTS_RE (1 << SDIO_RINTSTS_RE_SHIFT)

/* CDONE: Command done */
#define SDIO_RINTSTS_CDONE_SHIFT (2)
#define SDIO_RINTSTS_CDONE (1 << SDIO_RINTSTS_CDONE_SHIFT)

/* DTO: Data transfer over */
#define SDIO_RINTSTS_DTO_SHIFT (3)
#define SDIO_RINTSTS_DTO (1 << SDIO_RINTSTS_DTO_SHIFT)

/* TXDR: Transmit FIFO data request */
#define SDIO_RINTSTS_TXDR_SHIFT (4)
#define SDIO_RINTSTS_TXDR (1 << SDIO_RINTSTS_TXDR_SHIFT)

/* RXDR: Receive FIFO data request */
#define SDIO_RINTSTS_RXDR_SHIFT (5)
#define SDIO_RINTSTS_RXDR (1 << SDIO_RINTSTS_RXDR_SHIFT)

/* RCRC: Response CRC error */
#define SDIO_RINTSTS_RCRC_SHIFT (6)
#define SDIO_RINTSTS_RCRC (1 << SDIO_RINTSTS_RCRC_SHIFT)

/* DCRC: Data CRC error */
#define SDIO_RINTSTS_DCRC_SHIFT (7)
#define SDIO_RINTSTS_DCRC (1 << SDIO_RINTSTS_DCRC_SHIFT)

/* RTO_BAR: Response time-out (RTO)/boot ack received (BAR) */
#define SDIO_RINTSTS_RTO_BAR_SHIFT (8)
#define SDIO_RINTSTS_RTO_BAR (1 << SDIO_RINTSTS_RTO_BAR_SHIFT)

/* DRTO_BDS: Data read time-out (DRTO)/boot data start (BDS) */
#define SDIO_RINTSTS_DRTO_BDS_SHIFT (9)
#define SDIO_RINTSTS_DRTO_BDS (1 << SDIO_RINTSTS_DRTO_BDS_SHIFT)

/* HTO: Data starvation-by-host time-out */
#define SDIO_RINTSTS_HTO_SHIFT (10)
#define SDIO_RINTSTS_HTO (1 << SDIO_RINTSTS_HTO_SHIFT)

/* FRUN: FIFO underrun/overrun error */
#define SDIO_RINTSTS_FRUN_SHIFT (11)
#define SDIO_RINTSTS_FRUN (1 << SDIO_RINTSTS_FRUN_SHIFT)

/* HLE: Hardware locked write error */
#define SDIO_RINTSTS_HLE_SHIFT (12)
#define SDIO_RINTSTS_HLE (1 << SDIO_RINTSTS_HLE_SHIFT)

/* SBE: Start-bit error */
#define SDIO_RINTSTS_SBE_SHIFT (13)
#define SDIO_RINTSTS_SBE (1 << SDIO_RINTSTS_SBE_SHIFT)

/* ACD: Auto command done */
#define SDIO_RINTSTS_ACD_SHIFT (14)
#define SDIO_RINTSTS_ACD (1 << SDIO_RINTSTS_ACD_SHIFT)

/* EBE: End-bit error (read)/write no CRC */
#define SDIO_RINTSTS_EBE_SHIFT (15)
#define SDIO_RINTSTS_EBE (1 << SDIO_RINTSTS_EBE_SHIFT)

/* SDIO_INTERRUPT: Interrupt from SDIO card */
#define SDIO_RINTSTS_SDIO_INTERRUPT_SHIFT (16)
#define SDIO_RINTSTS_SDIO_INTERRUPT (1 << SDIO_RINTSTS_SDIO_INTERRUPT_SHIFT)

/* --- SDIO_STATUS values --------------------------------------- */

/* FIFO_RX_WATERMARK: FIFO reached receive watermark level */
#define SDIO_STATUS_FIFO_RX_WATERMARK_SHIFT (0)
#define SDIO_STATUS_FIFO_RX_WATERMARK (1 << SDIO_STATUS_FIFO_RX_WATERMARK_SHIFT)

/* FIFO_TX_WATERMARK: FIFO reached transmit watermark level */
#define SDIO_STATUS_FIFO_TX_WATERMARK_SHIFT (1)
#define SDIO_STATUS_FIFO_TX_WATERMARK (1 << SDIO_STATUS_FIFO_TX_WATERMARK_SHIFT)

/* FIFO_EMPTY: FIFO is empty */
#define SDIO_STATUS_FIFO_EMPTY_SHIFT (2)
#define SDIO_STATUS_FIFO_EMPTY (1 << SDIO_STATUS_FIFO_EMPTY_SHIFT)

/* FIFO_FULL: FIFO is full */
#define SDIO_STATUS_FIFO_FULL_SHIFT (3)
#define SDIO_STATUS_FIFO_FULL (1 << SDIO_STATUS_FIFO_FULL_SHIFT)

/* CMDFSMSTATES: Command FSM states */
#define SDIO_STATUS_CMDFSMSTATES_SHIFT (4)
#define SDIO_STATUS_CMDFSMSTATES_MASK (0xf << SDIO_STATUS_CMDFSMSTATES_SHIFT)
#define SDIO_STATUS_CMDFSMSTATES(x) ((x) << SDIO_STATUS_CMDFSMSTATES_SHIFT)

/* DATA_3_STATUS: Raw selected card_data[3] */
#define SDIO_STATUS_DATA_3_STATUS_SHIFT (8)
#define SDIO_STATUS_DATA_3_STATUS (1 << SDIO_STATUS_DATA_3_STATUS_SHIFT)

/* DATA_BUSY: Inverted version of raw selected card_data[0] */
#define SDIO_STATUS_DATA_BUSY_SHIFT (9)
#define SDIO_STATUS_DATA_BUSY (1 << SDIO_STATUS_DATA_BUSY_SHIFT)

/* DATA_STATE_MC_BUSY: Data transmit or receive state-machine is busy */
#define SDIO_STATUS_DATA_STATE_MC_BUSY_SHIFT (10)
#define SDIO_STATUS_DATA_STATE_MC_BUSY (1 << SDIO_STATUS_DATA_STATE_MC_BUSY_SHIFT)

/* RESPONSE_INDEX: Index of previous response */
#define SDIO_STATUS_RESPONSE_INDEX_SHIFT (11)
#define SDIO_STATUS_RESPONSE_INDEX_MASK (0x3f << SDIO_STATUS_RESPONSE_INDEX_SHIFT)
#define SDIO_STATUS_RESPONSE_INDEX(x) ((x) << SDIO_STATUS_RESPONSE_INDEX_SHIFT)

/* FIFO_COUNT: Number of filled locations in FIFO */
#define SDIO_STATUS_FIFO_COUNT_SHIFT (17)
#define SDIO_STATUS_FIFO_COUNT (1 << SDIO_STATUS_FIFO_COUNT_SHIFT)

/* DMA_ACK: DMA acknowledge signal */
#define SDIO_STATUS_DMA_ACK_SHIFT (30)
#define SDIO_STATUS_DMA_ACK (1 << SDIO_STATUS_DMA_ACK_SHIFT)

/* DMA_REQ: DMA request signal */
#define SDIO_STATUS_DMA_REQ_SHIFT (31)
#define SDIO_STATUS_DMA_REQ (1 << SDIO_STATUS_DMA_REQ_SHIFT)

/* --- SDIO_FIFOTH values --------------------------------------- */

/* TX_WMARK: FIFO threshold watermark level when transmitting data to card */
#define SDIO_FIFOTH_TX_WMARK_SHIFT (0)
#define SDIO_FIFOTH_TX_WMARK_MASK (0xfff << SDIO_FIFOTH_TX_WMARK_SHIFT)
#define SDIO_FIFOTH_TX_WMARK(x) ((x) << SDIO_FIFOTH_TX_WMARK_SHIFT)

/* RX_WMARK: FIFO threshold watermark level when receiving data from card */
#define SDIO_FIFOTH_RX_WMARK_SHIFT (16)
#define SDIO_FIFOTH_RX_WMARK_MASK (0xfff << SDIO_FIFOTH_RX_WMARK_SHIFT)
#define SDIO_FIFOTH_RX_WMARK(x) ((x) << SDIO_FIFOTH_RX_WMARK_SHIFT)

/* DMA_MTS: Burst size of multiple transaction */
#define SDIO_FIFOTH_DMA_MTS_SHIFT (28)
#define SDIO_FIFOTH_DMA_MTS_MASK (0x7 << SDIO_FIFOTH_DMA_MTS_SHIFT)
#define SDIO_FIFOTH_DMA_MTS(x) ((x) << SDIO_FIFOTH_DMA_MTS_SHIFT)

/* --- SDIO_CDETECT values -------------------------------------- */

/* CARD_DETECT: Card detect - 0 represents presence of card */
#define SDIO_CDETECT_CARD_DETECT_SHIFT (0)
#define SDIO_CDETECT_CARD_DETECT (1 << SDIO_CDETECT_CARD_DETECT_SHIFT)

/* --- SDIO_WRTPRT values --------------------------------------- */

/* WRITE_PROTECT: Write protect - 1 represents write protection */
#define SDIO_WRTPRT_WRITE_PROTECT_SHIFT (0)
#define SDIO_WRTPRT_WRITE_PROTECT (1 << SDIO_WRTPRT_WRITE_PROTECT_SHIFT)

/* --- SDIO_TCBCNT values --------------------------------------- */

/* TRANS_CARD_BYTE_COUNT: Number of bytes transferred by CIU unit to card */
#define SDIO_TCBCNT_TRANS_CARD_BYTE_COUNT_SHIFT (0)
#define SDIO_TCBCNT_TRANS_CARD_BYTE_COUNT_MASK (0xffffffff << SDIO_TCBCNT_TRANS_CARD_BYTE_COUNT_SHIFT)
#define SDIO_TCBCNT_TRANS_CARD_BYTE_COUNT(x) ((x) << SDIO_TCBCNT_TRANS_CARD_BYTE_COUNT_SHIFT)

/* --- SDIO_TBBCNT values --------------------------------------- */

/* TRANS_FIFO_BYTE_COUNT: Number of bytes transferred between host/DMA memory and BIU FIFO */
#define SDIO_TBBCNT_TRANS_FIFO_BYTE_COUNT_SHIFT (0)
#define SDIO_TBBCNT_TRANS_FIFO_BYTE_COUNT_MASK (0xffffffff << SDIO_TBBCNT_TRANS_FIFO_BYTE_COUNT_SHIFT)
#define SDIO_TBBCNT_TRANS_FIFO_BYTE_COUNT(x) ((x) << SDIO_TBBCNT_TRANS_FIFO_BYTE_COUNT_SHIFT)

/* --- SDIO_DEBNCE values --------------------------------------- */

/* DEBOUNCE_COUNT: Number of host clocks used by debounce filter logic for card detect */
#define SDIO_DEBNCE_DEBOUNCE_COUNT_SHIFT (0)
#define SDIO_DEBNCE_DEBOUNCE_COUNT_MASK (0xffffff << SDIO_DEBNCE_DEBOUNCE_COUNT_SHIFT)
#define SDIO_DEBNCE_DEBOUNCE_COUNT(x) ((x) << SDIO_DEBNCE_DEBOUNCE_COUNT_SHIFT)

/* --- SDIO_RST_N values ---------------------------------------- */

/* CARD_RESET: Hardware reset */
#define SDIO_RST_N_CARD_RESET_SHIFT (0)
#define SDIO_RST_N_CARD_RESET (1 << SDIO_RST_N_CARD_RESET_SHIFT)

/* --- SDIO_BMOD values ----------------------------------------- */

/* SWR: Software reset */
#define SDIO_BMOD_SWR_SHIFT (0)
#define SDIO_BMOD_SWR (1 << SDIO_BMOD_SWR_SHIFT)

/* FB: Fixed burst */
#define SDIO_BMOD_FB_SHIFT (1)
#define SDIO_BMOD_FB (1 << SDIO_BMOD_FB_SHIFT)

/* DSL: Descriptor skip length */
#define SDIO_BMOD_DSL_SHIFT (2)
#define SDIO_BMOD_DSL (1 << SDIO_BMOD_DSL_SHIFT)

/* DE: SD/MMC DMA enable */
#define SDIO_BMOD_DE_SHIFT (7)
#define SDIO_BMOD_DE (1 << SDIO_BMOD_DE_SHIFT)

/* PBL: Programmable burst length */
#define SDIO_BMOD_PBL_SHIFT (8)
#define SDIO_BMOD_PBL_MASK (0x7 << SDIO_BMOD_PBL_SHIFT)
#define SDIO_BMOD_PBL(x) ((x) << SDIO_BMOD_PBL_SHIFT)

/* --- SDIO_PLDMND values --------------------------------------- */

/* PD: Poll demand */
#define SDIO_PLDMND_PD_SHIFT (0)
#define SDIO_PLDMND_PD_MASK (0xffffffff << SDIO_PLDMND_PD_SHIFT)
#define SDIO_PLDMND_PD(x) ((x) << SDIO_PLDMND_PD_SHIFT)

/* --- SDIO_DBADDR values --------------------------------------- */

/* SDL: Start of descriptor list */
#define SDIO_DBADDR_SDL_SHIFT (0)
#define SDIO_DBADDR_SDL_MASK (0xffffffff << SDIO_DBADDR_SDL_SHIFT)
#define SDIO_DBADDR_SDL(x) ((x) << SDIO_DBADDR_SDL_SHIFT)

/* --- SDIO_IDSTS values ---------------------------------------- */

/* TI: Transmit interrupt */
#define SDIO_IDSTS_TI_SHIFT (0)
#define SDIO_IDSTS_TI (1 << SDIO_IDSTS_TI_SHIFT)

/* RI: Receive interrupt */
#define SDIO_IDSTS_RI_SHIFT (1)
#define SDIO_IDSTS_RI (1 << SDIO_IDSTS_RI_SHIFT)

/* FBE: Fatal bus error interrupt */
#define SDIO_IDSTS_FBE_SHIFT (2)
#define SDIO_IDSTS_FBE (1 << SDIO_IDSTS_FBE_SHIFT)

/* DU: Descriptor unavailable interrupt */
#define SDIO_IDSTS_DU_SHIFT (4)
#define SDIO_IDSTS_DU (1 << SDIO_IDSTS_DU_SHIFT)

/* CES: Card error summary */
#define SDIO_IDSTS_CES_SHIFT (5)
#define SDIO_IDSTS_CES (1 << SDIO_IDSTS_CES_SHIFT)

/* NIS: Normal interrupt summary */
#define SDIO_IDSTS_NIS_SHIFT (8)
#define SDIO_IDSTS_NIS (1 << SDIO_IDSTS_NIS_SHIFT)

/* AIS: Abnormal interrupt summary */
#define SDIO_IDSTS_AIS_SHIFT (9)
#define SDIO_IDSTS_AIS (1 << SDIO_IDSTS_AIS_SHIFT)

/* EB: Error bits */
#define SDIO_IDSTS_EB_SHIFT (10)
#define SDIO_IDSTS_EB_MASK (0x7 << SDIO_IDSTS_EB_SHIFT)
#define SDIO_IDSTS_EB(x) ((x) << SDIO_IDSTS_EB_SHIFT)

/* FSM: DMAC state machine present state */
#define SDIO_IDSTS_FSM_SHIFT (13)
#define SDIO_IDSTS_FSM_MASK (0xf << SDIO_IDSTS_FSM_SHIFT)
#define SDIO_IDSTS_FSM(x) ((x) << SDIO_IDSTS_FSM_SHIFT)

/* --- SDIO_IDINTEN values -------------------------------------- */

/* TI: Transmit interrupt enable */
#define SDIO_IDINTEN_TI_SHIFT (0)
#define SDIO_IDINTEN_TI (1 << SDIO_IDINTEN_TI_SHIFT)

/* RI: Receive interrupt enable */
#define SDIO_IDINTEN_RI_SHIFT (1)
#define SDIO_IDINTEN_RI (1 << SDIO_IDINTEN_RI_SHIFT)

/* FBE: Fatal bus error enable */
#define SDIO_IDINTEN_FBE_SHIFT (2)
#define SDIO_IDINTEN_FBE (1 << SDIO_IDINTEN_FBE_SHIFT)

/* DU: Descriptor unavailable interrupt */
#define SDIO_IDINTEN_DU_SHIFT (4)
#define SDIO_IDINTEN_DU (1 << SDIO_IDINTEN_DU_SHIFT)

/* CES: Card error summary interrupt */
#define SDIO_IDINTEN_CES_SHIFT (5)
#define SDIO_IDINTEN_CES (1 << SDIO_IDINTEN_CES_SHIFT)

/* NIS: Normal interrupt summary enable */
#define SDIO_IDINTEN_NIS_SHIFT (8)
#define SDIO_IDINTEN_NIS (1 << SDIO_IDINTEN_NIS_SHIFT)

/* AIS: Abnormal interrupt summary enable */
#define SDIO_IDINTEN_AIS_SHIFT (9)
#define SDIO_IDINTEN_AIS (1 << SDIO_IDINTEN_AIS_SHIFT)

/* --- SDIO_DSCADDR values -------------------------------------- */

/* HDA: Host descriptor address pointer */
#define SDIO_DSCADDR_HDA_SHIFT (0)
#define SDIO_DSCADDR_HDA_MASK (0xffffffff << SDIO_DSCADDR_HDA_SHIFT)
#define SDIO_DSCADDR_HDA(x) ((x) << SDIO_DSCADDR_HDA_SHIFT)

/* --- SDIO_BUFADDR values -------------------------------------- */

/* HBA: Host buffer address pointer */
#define SDIO_BUFADDR_HBA_SHIFT (0)
#define SDIO_BUFADDR_HBA_MASK (0xffffffff << SDIO_BUFADDR_HBA_SHIFT)
#define SDIO_BUFADDR_HBA(x) ((x) << SDIO_BUFADDR_HBA_SHIFT)

BEGIN_DECLS

/*****/

END_DECLS

/**@}*/

#ifdef __cplusplus
}
#endif

#endif
