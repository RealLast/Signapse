###############################################################################
# usage:   Replaces information in files using a CSV dictionary.              #
# author:  Robert Niklas Bock                                                 #
# created: 15.05.2018                                                         #
###############################################################################

import argparse
import pandas
import os.path
from tqdm import tqdm

from ensuring import ensure

CONSOLE_PRINT = "> {} --> {}"


###############################################################################
# PARSER

parser = argparse.ArgumentParser(
            description="Replaces information in CSV-files using a "
                        "CSV dictionary")

parser.add_argument("-s",
                    help="If given this flag, the script will run extremely "
                         "slow to ensure every entry has a translation.",
                    action='store_true')

parser.add_argument("-i",
                    "--input",
                    help="The file that shall be translated.",
                    required=True)

parser.add_argument("-c",
                    "--column",
                    help="The column in the input file where the information "
                         "should be translated.",
                    default="class")

parser.add_argument("-d",
                    "--dictionary",
                    help="The file which contains the translations which "
                         "should be used.",
                    required=True)

parser.add_argument("-f",
                    "--from_col",
                    help="The name of the column in the dictionary, which "
                         "contains the source names.",
                    default="from")

parser.add_argument("-t",
                    "--to_col",
                    help="The name of the column in the dictionary, which "
                         "contains the target names.",
                    default="to")

parser.add_argument("-o",
                    "--output",
                    help="The file where the result shall be written to.",
                    required=True)

parser.add_argument("-m",
                    "--meta",
                    help="Path to a txt file, where all lines are printed,"
                         "which might be errorous.",
                    required=True)

args = parser.parse_args()


###############################################################################
# VALIDATING FILES

# ensures validity of the input
ensure(os.path.isfile(args.input),
       "The input file does not exist.")
dataframe = pandas.read_csv(args.input, index_col=False)
ensure(args.column in dataframe.columns,
       "The translation column is not present.")

# ensures validity of the dictionary
ensure(os.path.isfile(args.dictionary),
       "The dictionary file does not exist.")
dictionary = pandas.read_csv(args.dictionary, index_col=False)
ensure(args.from_col in dictionary.columns,
       "The source column does not exist.")
ensure(args.to_col in dictionary.columns,
       "The target column does not exist.")


###############################################################################
# MAIN

def compare_in_and_out():
    same = None

    with open(args.input, 'r') as file1:
        with open(args.output, 'r') as file2:
            same = set(file1).intersection(file2)
            same.discard('\n')

    file1.close()
    file2.close()

    with open(args.meta, 'w') as file_out:
        for line in same:
            file_out.write(line)


def main():

    print "Reading from {} in column {}".format(args.input,
                                                args.column)
    print "Dictionary {} parses {} -> {}".format(args.dictionary,
                                                 args.from_col,
                                                 args.to_col)
    print "The output will be in {}".format(args.output)

    for i in tqdm(range(0, len(dictionary)), unit="labels"):
        dataframe[args.column].replace(dictionary[args.from_col][i],
                                       dictionary[args.to_col][i],
                                       inplace=True)

    # writing to file
    dataframe.to_csv(args.output, index=False)

    compare_in_and_out()


###############################################################################
# MAIN CHECK
if __name__ == "__main__":
    main()
