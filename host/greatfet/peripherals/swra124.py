#
# This file is part of GreatFET
#
import errno
import sys
import time

from intelhex import IntelHex

from ..peripheral import GreatFETPeripheral


class SWRA124(GreatFETPeripheral):
    execbuf = None
    execbufs = {0x0100: 0xF000,
                0x1100: 0xF000,
                0x8500: 0xF000,
                0x8900: 0xF000,
                0x8100: 0xF000,
                0x9100: 0xF000,
                0xA500: 0x0000,  # CC2530
                0xB500: 0x8000,
                0x9500: 0x8000,
                0x8D00: 0x8000,
                0xFF00: None}  # missing
    page_sizes = {0x01: 1024,  # "CC1110",
                  0x11: 1024,  # "CC1111",
                  0x85: 2048,  # "CC2430",
                  0x89: 2048,  # "CC2431",
                  0x81: 1024,  # "CC2510",
                  0x91: 1024,  # "CC2511",
                  0xA5: 2048,  # "CC2530", #page 57 of SWRU191B
                  0xB5: 2048,  # "CC2531",
                  0x95: 2048,  # "CC2533",
                  0x8D: 2048,  # "CC2540",
                  0xFF: None}
    statusbits = {0x80: "erase_done",
                  0x40: "pcon_idle",
                  0x20: "cpu_halted",
                  0x10: "pm0",
                  0x08: "halt_status",
                  0x04: "locked",
                  0x02: "oscstable",
                  0x01: "overflow"}
    configbits = {0x20: "soft_power_mode",   #new for CC2530
                  0x08: "timers_off",
                  0x04: "dma_pause",
                  0x02: "timer_suspend",
                  0x01: "sel_flash_info_page"}  #stricken from CC2530

    def __init__(self, board, log_function):
        """
            Initialize a new SWRA124 chipcon instance.

            Args:
                board -- The GreatFET board connected to the target.
        """
        self.board = board
        self.api = self.board.apis.swra124
        self.log_function = log_function

    def setup(self):
        self.api.setup()

    def debug_init(self):
        self.api.debug_init()

    def debug_stop(self):
        self.api.debug_stop()

    def chip_erase(self):
        self.log_function("Erasing chip..")
        self.api.chip_erase()
        self.api.read_status()
        while not (self.api.read_status() & 0x80):
            self.log_function(".")
            time.sleep(.1)
        self.log_function("Done erasing chip.")

    def read_status(self):
        status = self.api.read_status()
        self.log_function("status=%0.2x" % status)
        self.log_function(bin(status))
        str = ""
        i=1
        while i < 0x100:
            if status & i:
                str = "%s %s" % (self.statusbits[i], str)
            i *= 2
        return str

    def get_chip_id(self):
        return self.api.get_chip_id()

    def halt(self):
        self.api.halt()

    def resume(self):
        self.api.resume()

    def debug_instr(self):
        self.api.debug_instr()

    def step_instr(self):
        self.api.step_instr()

    def get_pc(self):
        return self.api.get_pc()

    def page_size(self):
        chip = self.get_chip_id() >> 8  # chip_id is type+ver, we only need type, so we shift out ver
        if chip == 0xFF:
            print("ERROR: Chip not found")
            sys.exit(errno.ENODEV)
        size = self.page_sizes.get(chip)
        if size is None or size < 1024:
            print("ERROR: Pagesize undefined.")
            print("chip=%0.4x" % chip)
            sys.exit(errno.ENODEV)

        return size

    def erase_flash_buffer(self):
        for i in range(0xf000, 0xf800):
            self.poke_data_byte(i, 0xFF)

    def write_flash_page(self, address):
        self.api.write_flash_page(address)

    def peek_code_byte(self, address):
        return self.api.peek_code_byte(address)

    def peek_data_byte(self, address):
        return self.api.peek_data_byte(address)

    def poke_data_byte(self, address, value):
        self.api.poke_data_byte(address, value)

    def oldflash(self, file):
        """Flash an intel hex file to code memory."""
        self.log_function("Flashing %s" % file)

        h = IntelHex(file)
        page = 0x0000
        pagelen = self.page_size()  # Varies by chip.
        if pagelen == 0:
            print("Page size 0, chip not found?")
            return
        self.log_function("page=%04x, pagelen=%04x" % (page, pagelen))

        # Wipe the RAM buffer for the next flash page.
        self.log_function("Erasing flash buffer..")
        self.erase_flash_buffer()
        bcount = 0
        for buf in h._buf.keys():
            while buf >= page + pagelen:
                if bcount > 0:
                    time.sleep(.1)  # Make sure last poke is processed by chip, wait a bit
                    self.write_flash_page(page)
                    bcount = 0
                    self.log_function("Flashed page at %06x" % page)
                page += pagelen

            # Place byte into buffer.
            # self.log_function("adr=%0.4x val=%0.2x" % (0xF000 + i - page, h[buf]))
            self.poke_data_byte(0xF000 + buf - page, h[buf])
            bcount += 1
            if buf % 0x100 == 0:
                self.log_function("Buffering %04x toward %06x" % (buf, page))
        # last page
        time.sleep(.1)  # Make sure last poke is processed by chip, wait a bit
        self.write_flash_page(page)
        self.log_function("Flashed final page at %06x" % page)

    def oldflashv2(self, file):
        """Flash an intel hex file to code memory."""
        self.log_function("Flashing %s" % file)

        h = IntelHex(file)
        page = 0x0000
        pagelen = self.page_size()  # Varies by chip.
        if pagelen == 0:
            print("Page size 0, chip not found?")
            return
        self.log_function("page=%04x, pagelen=%04x" % (page, pagelen))

        # Wipe the RAM buffer for the next flash page.
        self.log_function("Erasing flash buffer..")
        self.erase_flash_buffer()
        bcount = 0
        i = 0x0000
        for buf in h._buf.keys():
            while i >= page + pagelen:
                if bcount > 0:
                    self.write_flash_page(page)
                    bcount = 0
                    self.log_function("Flashed page at %06x" % page)
                page += pagelen

            # Place byte into buffer.
            # self.log_function("adr=%0.4x val=%0.2x" % (0xF000 + i - page, h[buf]))
            self.poke_data_byte(0xF000 + i - page, h[buf])
            bcount += 1
            i += 1
            if buf % 0x100 == 0:
                self.log_function("Buffering %04x toward %06x" % (buf, page))
        # last page
        self.write_flash_page(page)
        self.log_function("Flashed final page at %06x" % page)

    def flash(self, file):
        h = IntelHex(file)
        page = 0x0000
        pagelen = self.page_size()  # Varies by chip.
        if pagelen == 0:
            print("Page size 0, chip not found?")
            return
        self.log_function("page=%04x, pagelen=%04x" % (page, pagelen))

        # Wipe the RAM buffer for the next flash page.
        self.log_function("Erasing flash buffer..")
        self.erase_flash_buffer()
        bcount = 0
        hl = list(h._buf.keys())
        hl_len = len(hl)
        width = 16
        if hl:
            hl.sort()
            minaddr = hl[0]
            maxaddr = hl[-1]
            startaddr = (minaddr // width) * width
            endaddr = ((maxaddr // width) + 1) * width
            print("%0.4x %0.4x %0.4x" % (startaddr, endaddr, hl_len))
            #missing = 0
            bcount = 0
            i = 0
            for byteaddr in range(minaddr, maxaddr):
                addr = 0xF000 + i - page
                #data = 0xff
                if h[byteaddr]:
                    data = h[byteaddr]
                    self.poke_data_byte(addr, data)
                    #self.log_function("i=%0.4x addr=%0.4x data=%0.2x page=%0.4x" % (i, addr, data, page))
                    bcount += 1
                    if i >= page+pagelen-1:
                        time.sleep(.1)  # Make sure last poke is processed by chip, wait a bit
                        self.write_flash_page(page)
                        self.log_function("Flashed page at %06x" % page)
                        page += pagelen
                        bcount = 0
                    if byteaddr % 0x100 == 0:
                        self.log_function("Buffering %04x toward %06x" % (byteaddr, page))
                    i += 1
                #else:
                #    print("%0.4x missing filling fff" % i)
                #    missing += 1
            time.sleep(.1)  # Make sure last poke is processed by chip, wait a bit
            self.write_flash_page(page)
            self.log_function("Flashed page at %06x" % page)
            #print(missing)

    def oldverify(self, file):
        h = IntelHex(file)
        start = 0
        stop = 0xFFFF
        for buf in h._buf.keys():
            if(buf>=start and buf<stop):
                peek = self.peek_code_byte(buf)
                if(h[buf]!=peek):
                    print("ERROR at %04x, found %02x not %02x"%(buf,peek,h[buf]))
                if(buf%0x100==0):
                    print("%04x" % buf)

    def oldverifyv2(self, file):
        h = IntelHex(file)
        start = 0
        stop = 0xFFFF
        i = 0x0000
        for buf in h._buf.keys():
            if(i>=start and i<stop):
                peek = self.peek_code_byte(i)
                if(h[buf]!=peek):
                    print("ERROR at %04x, found %02x not %02x"%(i,peek,h[buf]))
                if(i%0x100==0):
                    print("%04x" % i)
            i += 1

    def verify(self, file):
        h = IntelHex(file)
        start = 0
        stop = 0xFFFF
        hl = list(h._buf.keys())
        hl_len = len(hl)
        width = 16
        if hl:
            hl.sort()
            minaddr = hl[0]
            maxaddr = hl[-1]
            startaddr = (minaddr // width) * width
            endaddr = ((maxaddr // width) + 1) * width
            print("%0.4x %0.4x %0.4x" % (startaddr, endaddr, hl_len))
            i = 0
            for byte in range(minaddr, maxaddr):
                data = 0xff
                peek = None
                if h[byte]:
                    data = h[byte]
                    if start <= i < stop:
                        peek = self.peek_code_byte(i)
                    if data != peek:
                        print("ERROR at %04x, found %02x not %02x" % (i, peek, data))
                    #else:
                    #    print("i=%04x peek=%02x data=%02x" % (i, peek, data))

                    if i % 0x100 == 0:
                        print("%04x" % i)
                    i += 1
