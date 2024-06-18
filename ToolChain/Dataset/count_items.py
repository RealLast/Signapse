###############################################################################
# author: Robert Niklas Bock                                                  #
# created: 2018-05-17                                                         #
###############################################################################

""" Counts unique entries in a CSV column and puts them in another file
"""

import os
import argparse
import pandas


###############################################################################
# PARSER

PARSER = argparse.ArgumentParser(description="Counts unique entries in a CSV "
                                             "column and puts them in another "
                                             "file")

PARSER.add_argument('-i', '--input', required=True)
PARSER.add_argument('-c', '--column', required=True)
PARSER.add_argument('-o', '--output', required=True)
ARGS = PARSER.parse_args()


def main():
    """ Main Logic of the script
    """

    if not os.path.isfile(ARGS.input):
        print "File doesnt exist."
        quit()

    df = pandas.read_csv(ARGS.input, index_col=False)

    if ARGS.column not in df.columns:
        print "File deosnt contain this column."
        quit()

    count_list = df[ARGS.column].value_counts()

    print count_list

    count_list.to_csv(ARGS.output)


###############################################################################
# MAIN CATCH

if __name__ == "__main__":
    main()
