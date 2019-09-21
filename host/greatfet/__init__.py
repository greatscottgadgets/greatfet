from __future__ import print_function
# Alias objects to make them easier to import.

from .greatfet import GreatFET
from .greatfet import GreatFETSingleton as GFSingleton
from .greatfet import GreatFETBoard


GreatFET = GreatFET  # pyflakes


class _GreatFETSingletonWrapper(object):

    """
    Convenience function that acts like GreatFETSingleton, but also allows Magic:
    accessing a property on this object will act as though that property had been
    accessed on a result of a GreatFETSingleton() call.

    That's heckin' unreadable, so in short-- accessing a property on a relevant object
    will attempt to 1) call the property on the sanest existing GreatFET object; or
    2) create a new GreatFET object, if necessary.
    """

    def __init__(self, serial=None):
        self.serial = serial

    def __getitem__(self, serial):
        return _GreatFETSingletonWrapper(serial)

    def __getattr__(self, name):
        return getattr(GFSingleton(self.serial), name)

    def __call__(self, serial=None):
        return GFSingleton(serial)

    def __dir__(self):
        return dir(GFSingleton(self.serial))

GreatFETSingleton = _GreatFETSingletonWrapper()


def greatfet_assets_directory():
    """ Provide a quick function that helps us get at our assets directory. """
    import os

    # Find the path to the module, and then find its assets folder.
    module_path = os.path.dirname(__file__)
    return os.path.join(module_path, 'assets')


def find_greatfet_asset(filename):
    """ Returns the path to a given GreatFET asset, if it exists, or None if the GreatFET asset isn't provided."""
    import os

    asset_path = os.path.join(greatfet_assets_directory(), filename)

    if os.path.isfile(asset_path):
        return asset_path
    else:
        return None

