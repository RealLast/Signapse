###############################################################################
# author:  Robert Niklas Bock                                                 #
# created: 15.05.2018                                                         #
###############################################################################

""" Allows to easily create progressbars in scripts
"""


import time
import sys

from ensuring import ensure

CONSOLE_PRINT = "\r[{}] {}% ({}/{})"


class ProgressBar(object):
    """ Creates a Progressbar that reprints itself on the console every time
        its percentage changes.
    """

    def __init__(self, total_ops, width=50, char_finished="#",
                 char_unfinished="-"):
        """ Constructor\n
            Set the total number of needed operations to acurately represent
            the progress of your task. Optionally you can set the width of the
            `ProgressBar` as well as the characters that are used to represent
            the finished and the unfinished part of it.
        """
        ensure(total_ops > 0, "The number of total operations has to be "
                              "positive.")

        self.total_operations = total_ops
        self.done_operations = 0
        self.width = width
        self.char_finished = char_finished
        self.char_unfinished = char_unfinished
        self.to_console()

    def _calc_percent(self):
        """ INTERNAL METHOD.\n
            Calculates which percentage of the task is done.
        """
        ratio = float(self.done_operations) / float(self.total_operations)
        return ratio * 100

    def _inner_string(self):
        """ INTERNAL METHOD.\n
            Generates the inner part of the progressbar depending on how much
            of it should be filled.
        """
        done_string = ""
        to_fill = int(self._calc_percent() * self.width / 100)

        for j in range(1, self.width):
            if j <= to_fill:
                done_string += self.char_finished
            else:
                done_string += self.char_unfinished
        return done_string

    def to_console(self):
        """ Prints the ProgressBar to the console.
        """
        sys.stdout.flush()
        sys.stdout.write(CONSOLE_PRINT.format(self._inner_string(),
                                              "{0:.2f}".format(self._calc_percent()),
                                              "",
                                              ""))

    def inc(self, increment=1):
        """ Counts up the operations of the task and reprints the
            `ProgressBar`. A number can be provided to increase the number of
            finished operations by that number.
        """
        self.done_operations += int(increment)
        self.to_console()


###############################################################################
# MAIN CHECK

if __name__ == "__main__":
    P = ProgressBar(100)
    for i in range(0, 100):
        P.inc(1)
        time.sleep(0.01)
