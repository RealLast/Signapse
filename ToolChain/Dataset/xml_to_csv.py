###############################################################################
# usage:   Converts LabelImg multifile XML output into single csv file        #
# author:  Soehnke Benedikt Fischedick                                         #
###############################################################################
""" Converts LabelImg multifile XML output into single file
"""
import os
import glob
import xml.etree.ElementTree as et
import argparse
import pandas as pd


# Coverts all xml files from path to pandas dataframe
def xml_to_csv(path):
    """ Covert every xml and add to pandas dataframe
    """
    xml_list = []
    for xml_file in glob.glob(path + '/*.xml'):
        tree = et.parse(xml_file)
        root = tree.getroot()
        for member in root.findall('object'):
            value = (root.find('filename').text,
                     int(root.find('size')[0].text),
                     int(root.find('size')[1].text),
                     member[0].text,
                     int(member[4][0].text),
                     int(member[4][1].text),
                     int(member[4][2].text),
                     int(member[4][3].text),)
            xml_list.append(value)
    column_name = ['filename', 'width', 'height', 'class',
                   'xmin', 'ymin', 'xmax', 'ymax']
    xml_df = pd.DataFrame(xml_list, columns=column_name)
    return xml_df


# Takes a folder with LableImg annotations and
# creats a pandas dataframe of this.
# This dataframe will be saved to a csv.
def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('-i', '--input', help='Input folder', required=True)
    parser.add_argument('-o', '--output', dest='output',
                        help='Output file', required=True)
    args = parser.parse_args()
    image_path = os.path.join(args.input)
    xml_df = xml_to_csv(image_path)
    xml_df.to_csv(args.output, index=None)
    print 'Done converting xml to csv.'


if __name__ == "__main__":
    main()
