###############################################################################
# author: Robert Niklas Bock                                                  #
# created: 2018-05-12                                                         #
###############################################################################

""" Tools for ensuring certain conditions without ugly if cascades.
"""


def ensure(condition, error_message="", critical=True):
    """ Ensures that a certain condition is met before subsequent code is
        executed. Conditions checked by ensure can be considered critical and
        will raise an Exception displaying an error message (if provided). If
        the condition is not critical the error message will be printed to
        console when not met. Under any circumstances ensure will return a
        bool telling whether the condition was met or not.
    """

    # type safing the condition
    if not isinstance(condition, bool):
        raise Exception("condition is not of type bool; "
                        "type of: {}".format(str(type(condition))))

    # type safing the message
    if not isinstance(error_message, str):
        raise Exception("error_message is not of type str; "
                        "type of: {}".format(str(type(error_message))))

    # checking the condition
    if not condition and critical is True:
        raise Exception(error_message)

    if not condition and critical is False:
        print "[ERR] " + error_message

    return condition
