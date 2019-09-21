
from os.path import dirname, basename, isfile
import glob

# Autodetect all modules in this directory, so they can be automatically imported by 'from <> import *'.
module_paths = glob.glob(dirname(__file__)+"/*.py")
__all__ = [ basename(f)[:-3] for f in module_paths if isfile(f)]
