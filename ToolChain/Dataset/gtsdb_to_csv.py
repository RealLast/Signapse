###############################################################################
# usage:   Converts GTSDB labels format to our own                            #
# author:  Soehnke Benedikt Fischedick                                         #
###############################################################################
""" Used to convert gtsdb labels to own format
"""
import argparse
import pandas as pd


# Converts GTSDB lables to CSV with more data(e.g. width and height)
def gtsdb_to_csv(path):
    """ Converts GTSDB to CSV style
    """
    gt_df = pd.read_csv(path, sep=";", header=None)
    gt_df.columns = ['filename', 'xmin', 'ymin', 'xmax', 'ymax', 'class']
    gt_df['width'] = 1360
    gt_df['height'] = 800
    gt_df = gt_df[['filename', 'width', 'height', 'class',
                   'xmin', 'ymin', 'xmax', 'ymax']]
    return gt_df


# Converts GTSDB to CSV and replace sign classification to higer classes.
# 0 is prohibitory
# 1 is danger
# 2 is mandatory
# 3 is other
# This is not used atm. maybe used later cause
# detection dataset will be single class
def gtsdb_to_csv_gen_classes(path):
    """ Converts GTSDB to CSV style and merge classes
    """
    gt_df = gtsdb_to_csv(path)
    # Class types from GTSDB. Readme for more details
    gt_df['class'].replace([0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 15, 16],
                           "prohibitory", True)
    gt_df['class'].replace([11, 18, 19, 20, 21, 22, 23, 24,
                            25, 26, 27, 28, 29, 30, 31], "danger", True)
    gt_df['class'].replace([33, 34, 35, 36, 37, 38, 39, 40], "mandatory", True)
    gt_df['class'].replace([0, 12, 13, 14, 17, 32, 41, 42], "other", True)
    return gt_df


# Takes arguments, creats csv dataframe with gtsdb_to_csv
# methode and saves it to csv
def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('-i', '--input', help='Input file', required=True)
    parser.add_argument('-o', '--output', dest='output',
                        help='Output file', required=True)
    args = parser.parse_args()
    lable_path = args.input
    csv_df = gtsdb_to_csv(lable_path)
    csv_df.to_csv(args.output, index=None)


if __name__ == "__main__":
    main()
