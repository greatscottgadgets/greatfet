#
# This file is part of GreatFET.
#
"""
Utilities for running GreatFET code interactively in IPython.
"""

import os
import time
import greatfet
import multiprocessing

from IPython.lib.deepreload import reload as deepreload
from IPython.core.magic import Magics, magics_class, line_magic, cell_magic, line_cell_magic


@magics_class
class GreatFETShellMagics(Magics):
    """ Class that provides convenience magics for running GreatFET shells. """

    def gf(self):
        return self.shell.user_ns['gf']


    @line_magic
    def reconnect(self, line):
        """ Reconnects to the active GreatFET, when possible. """

        # Try to reconnect to the attached GreatFET.
        self.gf().try_reconnect()


    def _get_greatfet_build_dir_arg(self):
        """ Returns any make argument necessary to change to the user's build directory; based on user hints. """

        # If we have any user context hints telling us where to find the GreatFET build directory, use it.
        if 'GREATFET_BUILD_DIR' in self.shell.user_ns:
            return '-C{}'.format(self.shell.user_ns['GREATFET_BUILD_DIR'])
        elif 'GREATFET_BUILD_DIR' in os.environ:
            return '-C{}'.format(os.environ['GREATFET_BUILD_DIR'])
        else:
            return ""


    @line_magic
    def make(self, line):
        """ Runs 'make <arg>', and then attempts to reconnect to the given GreatFET. """

        # Execute the make/flash...
        command = 'make -j{} {} {}'.format(multiprocessing.cpu_count(), self._get_greatfet_build_dir_arg(), line)
        self.shell.system(command)

        # ... give the device a chance to restart ...
        time.sleep(5)

        # ... and try to reconnect to the given GreatFET.
        self.gf().try_reconnect()


    @line_magic
    def makeflash(self, line):
        """ Runs 'make greatfet_usb-flash' to reflash the current board. """

        # Execute the make/flash...
        command = 'make greatfet_usb-flash -j{} {} {}'.format(multiprocessing.cpu_count(), self._get_greatfet_build_dir_arg(), line)
        self.shell.system(command)

        # ... give the device a chance to restart ...
        time.sleep(5)

        # ... and try to reconnect to the given GreatFET.
        self.gf().try_reconnect()


    @line_magic
    def reload(self, line):
        """ Attempts to reload any modules that have changed. Wrapper for %autoreload. """

        try:
            self.shell.run_magic('autoreload', silent=True)
        except:
            pass


    @line_magic
    def dmesg(self, line):
        """ Print's the GreatFET's debug ring. """
        self.gf().dmesg()


    @line_magic
    def refet(self, line):
        """ Resets the attached GreatFET, and then reconnects. Hey, %reset was taken. """

        self.gf().reset(reconnect=True)


    @line_magic
    def reset(self, line):
        """Resets the namespace by removing all names defined by the user, if
        called without arguments, or by removing some types of objects, such
        as everything currently in IPython's In[] and Out[] containers (see
        the parameters for details).
        Parameters
        ----------
        -f : force reset without asking for confirmation.
        -r : reset the attached GreatFET board, as well
        -s : 'Soft' reset: Only clears your namespace, leaving history intact.
            References to objects may be kept. By default (without this option),
            we do a 'hard' reset, giving you a new session and removing all
            references to objects from the current session.

        in : reset input history
        out : reset output history
        dhist : reset directory history
        array : reset only variables that are NumPy arrays
        """


        #
        #  We duplicate the existing %reset code here. Hey, it's BSD.
        #  This code contains stuff that's Copyright (c) 2012 The IPython Development Team.
        #

        # Confirm, as we would for the parent reset.
        opts, args = self.parse_options(line,'sfr', mode='list')

        if 'f' in opts:
            ans = True
        else:
            try:
                ans = self.shell.ask_yes_no(
                "Once deleted, variables cannot be recovered. Proceed (y/[n])?",
                default='n')
            except StdinNotImplementedError:
                ans = True
        if not ans:
            print('Nothing done.')
            return


        if 'r' in opts:
            reset_greatfet = True
        else:
            try:
                reset_greatfet = self.shell.ask_yes_no(
                "Would you like to reset the GreatFET hardware, as well (y/[n])?",
                default='n')
            except StdinNotImplementedError:
                reset_greatfet = False


        # If we need to reset the GreatFET, do so.
        if reset_greatfet:
            print("Resetting GreatFET.")
            self.gf().reset()

        # Call our inner reset...
        if 's' in opts:                     # Soft reset
                user_ns = self.shell.user_ns
                for i in self.who_ls():
                    del(user_ns[i])
        elif len(args) == 0:                # Hard reset
                self.shell.reset(new_session = False)


        # ... and then reconnect to the GreatFET.
        gf = self.shell.connect_function()
        self.shell.push('gf')


