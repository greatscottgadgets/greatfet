#
# This file is part of GreatFET
#

class GlitchKitCollection(object):
    """
    An object that has a reference to all supported GlitchKit modules.
    """

    def __init__(self, board):
        """
        Creates a new GlitchKit collection.

        Args:
            board -- The board for which the GlitchKit module can be populated.
        """

        for cls in GlitchKitModule.__subclasses__():
            if cls.supports_board(board):

                # If we're able to automatically instantiate this module, do so.
                try:
                    self.__dict__[cls.SHORT_NAME] = cls(board)
                except TypeError:
                    pass



class GlitchKitModule(object):
    """
    Generic base class for GlitchKit modules.
    """

    # The short name for the given module.
    SHORT_NAME = None

    @classmethod
    def supports_board(cls, board):
        """ Returns true iff the given class can be instantiated as a peripheral of the given board. """
        return True
