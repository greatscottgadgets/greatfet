
#
# This file is part of GreatFET.
#

class GreatFETNeighbor(object):
    """ Base class for objects represneting neighbor boards. """

    # Optional: subclasses can set this variable to override their neighbor name.
    # If not provided, their name will automatically be taken from their class names.
    # This typically doesn't need to be overridden.
    NEIGHBOR_NAME = None


    @classmethod
    def get_name(cls):
        """ Default implementation of a function that returns a class's name. """

        # If we have an overridden neighbor name, return it.
        if cls.NEIGHBOR_NAME:
            return cls.NEIGHBOR_NAME

        # Otherwise, return the given class's name.
        return cls.__name__


    @classmethod
    def available_neighbors(cls):
        """ Returns a list of available neighbors. """
        return [neighbor.get_name() for neighbor in cls.__subclasses__()]



    @classmethod
    def from_name(cls, name, board, *args, **kwargs):
        """ Creates a new GreatFETNeighbor object from its name. """

        target_name = name.lower()

        for subclass in cls.__subclasses__():

            # Grab the class's name, and check to see if it matches ours.
            subclass_name = subclass.get_name()

            # If this class matches our target name, this is the class we're looking for!
            # Create an instance and return it.
            if target_name == subclass_name.lower():
                return subclass(board, *args, **kwargs)


        raise ValueError("No known driver for neighbor '{}'.".format(name))

