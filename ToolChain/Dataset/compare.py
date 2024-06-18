###############################################################################
# author:  Robert Niklas Bock                                                 #
# created: 21.05.2018                                                         #
###############################################################################

""" Compares two files line by line
"""

import argparse

###############################################################################
# PARSER

PARSER = argparse.ArgumentParser(description="Compares two files line by line")

PARSER.add_argument("-ff",
                    "--firstfile",
                    help="one of the files that should be compared",
                    required=True)

PARSER.add_argument("-sf",
                    "--secondfile",
                    help="one of the files that should be compared",
                    required=True)

PARSER.add_argument("-o",
                    "--output",
                    help="name of the output file",
                    required=True)

ARGS = PARSER.parse_args()


###############################################################################
# MAIN

def main():
    """ Main Logic of this script
    """

    # list of identical lines
    same = None

    # read files and find identical lines
    with open(ARGS.firstfile, 'r') as file1:
        with open(ARGS.secondfile, 'r') as file2:
            same = set(file1).intersection(file2)
            same.discard('\n')

    # close files
    file1.close()
    file2.close()

    # write new file
    with open(ARGS.output, 'w') as file_out:
        for line in same:
            file_out.write(line)


###############################################################################
# MAIN CATCH

if __name__ == "__main__":
    main()
