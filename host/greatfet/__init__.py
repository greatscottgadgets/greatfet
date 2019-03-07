from __future__ import print_function
# Alias objects to make them easier to import.
from .greatfet import GreatFET
GreatFET = GreatFET  # pyflakes

def greatfet_assets_directory():
    """ Provide a quick function that helps us get at our assets directory. """
    import os

    # Find the path to the module, and then find its assets folder.
    module_path = os.path.dirname(__file__)
    return os.path.join(module_path, 'assets')
