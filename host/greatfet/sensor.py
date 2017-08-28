#
# This file is part of GreatFET
#

class GreatFETSensor(object):
    """
        Class representing an sensor that can be attached to a GreatFET board;
        or which is provided by a neighbor.

        Subclasses should support "get_reading" and "create_sensor".
    """

    # The 'shorthand' name for the sensor, which is used by command-line tools 
    # to identify the given type of sensor.
    SHORTHAND = ""

    def create_sensor(self, board, options):
        """
        Factory method that creates a sensor.

        Args:
            board -- The GreatFET being configured using the sensor API.
            options -- A dictionary of any options to be provided to the device.
                To be used e.g. on application command lines.
        """
        raise NotImplementedError("Attempted to create an abstract sensor!")


    def get_reading(self, name_prefix=True):
        """
            Returns a sensor reading for a configured sensor.
            Should be overridden by each sensor object.
        """
        return {}


