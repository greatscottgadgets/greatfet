from os.path import dirname, basename, isfile
import glob

# Autodetect all modules in the modules directory, so they can be automatically
# imported by 'from glitchkit import *'.
# TODO: Dedupclicate with code in the boards/__init__.
modules = glob.glob(dirname(__file__)+"/*.py")
__all__ = [ basename(f)[:-3] for f in modules if isfile(f)]

# Allow the base objects to be referred to without their ".base" prefix.
from .base import GlitchKitCollection, GlitchKitModule
__all__.extend(["GlitchKitCollection", "GlitchKitModule"])
