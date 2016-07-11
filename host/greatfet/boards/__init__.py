
from os.path import dirname, basename, isfile
import glob

# Autodetect all boards in the boards directory, so they can be automatically
# imported by 'from boards import *'. This allows easy development of new
# boards-- just create your boards/ file in boards, and be sure to subclass
# GreatFETBoard.
modules = glob.glob(dirname(__file__)+"/*.py")
__all__ = [ basename(f)[:-3] for f in modules if isfile(f)]
