#
# This file is part of GreatFET
#
"""
Vendor request numbers-- used for sending USB control requests to
GreatFET boards for non-bulk communication. These are still tenative.

When a final set is created, these should separate global requests (e.g.
requests that every GreatFET base should implement) from per-base requests.

Ideally we'll add an offset (100?) to per-board vendor requests to separate
requests that may differ from base-board to base-board.
"""

requests = (
    # Internal programming requests.
    'SPIFLASH_INIT',
    'SPIFLASH_WRITE',
    'SPIFLASH_READ',
    'SPIFLASH_ERASE',

    # Board information API.
    'READ_BOARD_ID',
    'READ_VERSION_STRING',
    'READ_PARTID_SERIALNO',

    # Temporary, custom stuffs?
    'ENABLE_USB1',
    'SET_LEDS',

    'GPIO_REGISTER',
    'GPIO_WRITE',

    'SPI_INIT',
    'SPI_WRITE',
    'SPI_READ',
    'SPI_DUMP_FLASH',

    'I2C_START',
    'I2C_STOP',
    'I2C_XFER',
    'I2C_RESPONSE',
    'I2C_GET_STATUS',

    'LOGIC_ANALYZER_START',
    'LOGIC_ANALYZER_STOP',

    'RESET',

    'ADC_INIT',
    'ADC_READ',
    'ADC_STREAM',

    'SDIR_RX_START',
    'SDIR_RX_STOP',
    'SDIR_TX_START',
    'SDIR_TX_STOP',

    'GREATDANCER_CONNECT',
    'GREATDANCER_DISCONNECT',
    'GREATDANCER_BUS_RESET',
    'GREATDANCER_SET_ADDRESS',
    'GREATDANCER_SET_UP_ENDPOINTS',

    'GREATDANCER_GET_STATUS',
    'GREATDANCER_READ_SETUP',
    'GREATDANCER_STALL_ENDPOINT',
    'GREATDANCER_SEND_ON_ENDPOINT',
    'GREATDANCER_CLEAN_UP_TRANSFER',
    'GREATDANCER_START_NONBLOCKING_READ',
    'GREATDANCER_FINISH_NONBLOCKING_READ',
    'GREATDANCER_GET_NONBLOCKING_LENGTH',

    'HEARTBEAT_START',
    'HEARTBEAT_STOP',

    'GPIO_RESET',
    'GPIO_READ',


    'GLITCHKIT_SETUP',
    'GLITCHKIT_PROVIDE_TARGET_CLOCK',
    'GLITCHKIT_SIMPLE_START_EVENT_COUNT',
    'GLITCHKIT_USB_CONTROL_IN_START',
    'GLITCHKIT_USB_RESULT_LENGTH',
    'GLITCHKIT_USB_READ_RESULT',

    'USBHOST_CONNECT',
    'USBHOST_BUS_RESET',
    'USBHOST_GET_STATUS',
    'USBHOST_SET_UP_ENDPOINT',
    'USBHOST_SEND_ON_ENDPOINT',
    'USBHOST_START_NONBLOCKING_READ',
    'USBHOST_FINISH_NONBLOCKING_READ',
    'USBHOST_GET_NONBLOCKING_LENGTH',
    
    'SUPER_HACKY',

    'DS18B20_READ',
    #'MSP430_JTAG',

    'DAC_SET',

    'READ_DMESG'
)

def _create_module_level_constants():
    this_module = globals()
    for i, name in enumerate(this_module['requests']):
        this_module[name] = i
_create_module_level_constants()
