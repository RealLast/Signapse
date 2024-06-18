""" Delete ranges of files in given CSV
"""
import os
import argparse
import pandas as pd


# check if delete.csv exists
def check_io(args):
    """ check if arg is correct
    """
    if not os.path.isdir(args.input):
        print 'Folder not found!'
    if not os.path.isfile(args.input + '/delete.csv'):
        print 'Can\'t find delete.csv'
    else:
        delete(args)


# will load delete.csv and delete range in it
def delete(args):
    """ Delete files from CSV
    """
    csv = pd.read_csv(args.input + '/delete.csv')
    for _, row in csv.iterrows():
        print row['from'], row['to']
        for x in range(row['from'], row['to']):
            if os.path.isfile(args.input+'/frame' + str(x) + '.png'):
                os.remove(args.input+'/frame' + str(x) + '.png')


# Used for agurment parsing. After that check_io will get called
def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('-i', '--input', help='Input folder', required=True)
    args = parser.parse_args()
    check_io(args)


if __name__ == "__main__":
    main()
