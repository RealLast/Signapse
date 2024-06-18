###############################################################################
# author:  Robert Niklas Bock                                                 #
# created: 25.05.2018                                                         #
###############################################################################

""" Converts PPMs to PNGs and puts them in a nice directory structure while
    providing a single csv for meta data.
"""

import argparse
import os
import pandas
from PIL import Image
from tqdm import tqdm

from ensuring import ensure


###############################################################################
# FUNCTIONS

def main():
    args = get_args()

    if args.r:
        dict_used = args.dict is not None
        df = pandas.read_csv(args.dict, index_col=False)

    all_csvs = []
    # loops through each subfolder
    if args.r:
        items = os.walk(args.output)
    else:
        items = [next(os.walk(args.input))]

    for root, dirs, files in items:
        index = 0

        if not args.r:
            for file in tqdm(files, unit="files"):
                if file.endswith(".ppm"):
                    new_name = os.path.splitext(file)[0] + ".png"
                    im = Image.open(os.path.join(root, file))
                    im.save(os.path.join(args.output, new_name))

        if args.r:
            for directory in dirs:
                num = int(directory)
                folder = (df.loc[df['from'] == num]['to']).unique()[0]

                new_path = os.path.join(args.output, folder)
                if not os.path.isdir(new_path):
                    os.makedirs(new_path)

                # finds each file per subfolder
                for sroot, _, sfiles in os.walk(os.path.join(root, directory)):
                    for name in tqdm(sfiles, unit="files"):

                        # ppms are converted to pngs
                        if name.endswith(".ppm"):
                            im = Image.open(os.path.join(sroot, name))

                            new_file = (str(index) + "_" + os.path
                                        .splitext(name)[0] + ".png")
                            im.save(os.path.join(new_path, new_file))
                            im.save(os.path.join(args.output, new_file))

                        # csvs are appended to the complete collection
                        if name.endswith(".csv") and dict_used:
                            csv_path = os.path.join(sroot, name)
                            csv = pandas.read_csv(csv_path, sep=";")
                            csv['ClassId'] = folder
                            csv['Filename'] = csv['Filename'].apply(filename_edit,
                                                                    args=(index,))
                            all_csvs.append(csv)
                            dframe = pandas.concat(all_csvs)
                            dframe.to_csv(os.path.join(args.output, "labels.csv"),
                                          index=False)
        index += 1


# little workaround for pandas' apply-function
def filename_edit(x, i):
    ''' Changes the filename
    '''
    result = str(i) + "_" + x.replace(".ppm", ".png")
    return result


def get_args():
    """ Reads arguments and check if they are correct
    """
    parser = argparse.ArgumentParser(
        description="Converts PPMs to PNGs and puts them in a nice "
                    "directory structure while providing a single csv "
                    "for meta data.")

    parser.add_argument("-i",
                        "--input",
                        help="Directory of the dataset",
                        required=True)

    parser.add_argument("-o",
                        "--output",
                        help="Target Directory",
                        required=True)

    parser.add_argument("-d",
                        "--dict",
                        help="path to the dictonary file")

    parser.add_argument("-r",
                        help="whether subfolders should be checked and "
                             "created",
                        action="store_true")

    args = parser.parse_args()

    if args.r and not args.dict:
        print "if r is given, dict is needed"
        quit()

    ensure(os.path.isdir(args.input), "Input is not a directory")

    if args.dict:
        ensure(os.path.isfile(args.dict), "Dictionary is not a file")

    make_dir = not ensure(os.path.isdir(args.output),
                          "Output is not a directory. Creating it.",
                          False)
    if make_dir:
        os.makedirs(args.output)

    return args


###############################################################################
# MAIN CATCH

if __name__ == "__main__":
    main()
