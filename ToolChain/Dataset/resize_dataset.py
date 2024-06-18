###############################################################################
# usage:   Rescales given dataset and labled boxes                            #
# author:  Soehnke Benedikt Fischedick                                         #
###############################################################################
""" Resize images from dataset to given size
"""
import argparse
import os.path
import cv2
import pandas as pd
import numpy as np

from progressbar import ProgressBar


def rescale_box(scale_x, scale_y, df, index, output_df):
    """ Rescales a box.
    """

    width_scaled = int(df['width'][index] * scale_x)
    height_scaled = int(df['height'][index] * scale_y)
    x_min = int(df['xmin'][index] * scale_x)
    y_min = int(df['ymin'][index] * scale_y)
    x_max = int(df['xmax'][index] * scale_x)
    y_max = int(df['ymax'][index] * scale_y)
    # Thorugh resizeing some signs will be to small for detection
    # Therfor we try to use thresholds
    # Our first attempt is a threshold based on the area of the box
    threshold_rect = 55  # To-Do find correct value
    box_area = int((x_max-x_min) * (y_max-y_min))
    area_ok = box_area >= threshold_rect
    # 2. width is not to small
    threshold_width = 8
    width_ok = (x_max - x_min) >= threshold_width
    # 3. height is not to small
    threshold_height = 8
    height_ok = (y_max - y_min) >= threshold_height
    normal_ok = area_ok and width_ok and height_ok
    additional_ok = False
    if df['class'][index] == 'additional' and width_ok:
        additional_ok = True
    if normal_ok or additional_ok:
        output_df.loc[index] = [df['filename'][index], width_scaled,
                                height_scaled, df['class'][index],
                                x_min, y_min, x_max, y_max]


def rescale_image(df, index, args, output_df):
    """ rescale image and box and save
    """
    max_size = float(args.size)
    file_path = os.path.join(args.input, df['filename'][index])
    if not os.path.isfile(file_path):
        print 'Image ' + file_path + ' not found!'
        return False
    img = cv2.imread(file_path)
    # Rescale factor needed cause images have different size
    # Scaling to 512 pixel cause it will be network input size
    width, height = img.shape[1::-1]

    scale_factor_x = max_size/float(width)
    scale_factor_y = max_size/float(height)
    resized_img = cv2.resize(img, (0, 0), fx=scale_factor_x,
                             fy=scale_factor_y, interpolation=cv2.INTER_CUBIC)

    output_file = os.path.join(args.output, df['filename'][index])
    # Only write file, if not previously written.
    # Maybe there are two signs in a image and else we would save it twice.
    if not os.path.isfile(output_file):
        cv2.imwrite(output_file, resized_img, [cv2.IMWRITE_PNG_COMPRESSION, 0])
    rescale_box(scale_factor_x, scale_factor_y, df, index, output_df)


def check_csv(df):
    """ Check if CSV matches layout
    """
    if ('xmin' in df.columns and 'ymin' in df.columns and
            'xmax' in df .columns and 'ymax' in df.columns and
            'class' in df.columns and 'filename' in df.columns):
        return True
    else:
        return False


def parse_args():
    """ Parses arguments and check if they are correct
    """
    parser = argparse.ArgumentParser()
    parser.add_argument('-i', '--input',
                        help='Input folder with images',
                        required=True)
    parser.add_argument('-l', '--labels',
                        help='Input CSV with lables',
                        required=True)
    parser.add_argument('-o', '--output',
                        help='Output folder',
                        required=True)
    parser.add_argument('-s', '--size',
                        help="New size after image scaled",
                        required=True)
    parser.add_argument('-d', help="internal debug flag", action='store_true')
    args = parser.parse_args()
    if not os.path.isdir(args.input):
        print 'Input folder not found'
        exit()
    if not os.path.isfile(args.labels):
        print 'CSV with labels not found'
        exit()
    if not os.path.isdir(args.output):
        print 'Output folder not found!'
        print 'Going to create one.'
        os.makedirs(args.output)
    return args


def main():
    args = parse_args()
    labels_df = pd.read_csv(args.labels)
    # Create dataframe matching to GTSRB dataset layout
    output_df = pd.DataFrame(columns=['filename', 'width', 'height', 'class',
                                      'xmin', 'ymin', 'xmax', 'ymax'])
    if not check_csv(labels_df):
        print 'CSV with labels is not in expected format'
        exit()
    p_bar = ProgressBar(len(labels_df))
    p_bar.to_console()
    for i, _ in labels_df.iterrows():
        p_bar.inc()
        if not rescale_image(labels_df, i, args, output_df) and args.d:
            continue
    output_df.to_csv(os.path.join(args.output, 'labels.csv'), index=False)


if __name__ == "__main__":
    main()
