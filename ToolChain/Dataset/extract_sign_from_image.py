###############################################################################
# usage:   From given dataset and lables, it will extract the sign(box)       #
# author:  Soehnke Benedikt Fischedick                                         #
###############################################################################
""" Extract rect sign box from given image in CSV
"""
import os.path
import argparse
import math
import cv2
import pandas as pd


from tqdm import tqdm

# Controls if GTSRB border is used or not, caus the is nor argparse option yet.
NO_BORDER = False


def check_if_file_exists(path):
    """ Returns true if path is valid file
    """
    if not os.path.isfile(path):
        return False
    else:
        return True


def check_if_folder_exists(path):
    """ Returns true if path is valid folder
    """
    if not os.path.isdir(path):
        return False
    else:
        return True


def check_if_lables_df_is_correct(labels_df):
    """ Check if Dataframe matches the rquiered one
    """
    if ('xmin' in labels_df.columns and 'ymin' in labels_df.columns and
            'xmax' in labels_df .columns and 'ymax' in labels_df.columns and
            'class' in labels_df.columns and 'filename' in labels_df.columns):
        return True
    else:
        return False


def crop_image_and_save(labels_df, index, output_df):
    """ Crope image to size from CSV
    """
    file_path = os.path.join(ARGS.input, labels_df['filename'][index])
    img = cv2.imread(file_path)

    # Get image size for setting correct border
    img_width, img_height = img.shape[1::-1]
    xmin = labels_df['xmin'][index]
    xmax = labels_df['xmax'][index]
    ymin = labels_df['ymin'][index]
    ymax = labels_df['ymax'][index]
    # To do crop the images same as GTSRB we set the box a bit bigger
    # GTSRB set border at least 5 pixel or 10 percent
    height = ymax - ymin
    width = xmax - xmin
    x_border_size = width * 1.1 - width

    if x_border_size < 10:
        if xmin - 5 > 0:
            xmin_border = xmin - 5
        else:
            xmin_border = 0
        if xmax + 5 < img_width:
            xmax_border = xmax + 5
        else:
            xmax_border = xmax
    else:
        if int(xmin - x_border_size/2) > 0:
            xmin_border = int(math.ceil(xmin - x_border_size/2))
        else:
            xmin_border = 0
        if int(xmax + x_border_size/2) < img_width:
            xmax_border = int(math.ceil(xmax + x_border_size/2))
        else:
            xmax_border = img_width

    y_border_size = height * 1.1 - height
    if y_border_size < 10:
        if ymin - 5 > 0:
            ymin_border = ymin - 5
        else:
            ymin_border = 0

        if ymax + 5 < img_height:
            ymax_border = ymax + 5
        else:
            ymax_border = img_height
    else:
        if int(ymin - y_border_size/2) > 0:
            ymin_border = int(math.ceil(ymin - y_border_size/2))
        else:
            ymin_border = 0
        if int(ymax + y_border_size/2) < img_height:
            ymax_border = int(math.ceil(ymax + y_border_size/2))
        else:
            ymax_border = img_height

    if NO_BORDER:
        crop_img = img[ymin:ymax, xmin:xmax]
    else:
        crop_img = img[ymin_border:ymax_border, xmin_border:xmax_border]
    # Saving two times.
    # One in the args defined output folder and second in a extra class folder.
    filename = str(index) + "_" + labels_df['filename'][index]
###############################################################################
# comment when using
    # cv2.imwrite(os.path.normpath(os.path.join(argssy.output, filename)),
    #             crop_img, [cv2.IMWRITE_PNG_COMPRESSION, 0])
    sub_folder_path = os.path.normpath(os.path.join(ARGS.output,
                                                    labels_df['class'][index]))
    if not check_if_folder_exists(sub_folder_path):
        os.makedirs(sub_folder_path)
    cv2.imwrite(os.path.normpath(os.path.join(sub_folder_path, filename)),
                crop_img, [cv2.IMWRITE_PNG_COMPRESSION, 0])
    width = xmax_border-xmin_border
    height = ymax_border-ymin_border

    xmax = width - xmax_border + xmax
    xmin = width - xmax

    ymax = height - ymax_border + ymax
    ymin = height - ymax
    output_df.loc[index] = [filename, width, height, xmin,
                            ymin, xmax, ymax, labels_df['class'][index],
                            0, 0, 0]


def parse_args():
    """ Use to parse arguments and check if they are valid
    """
    parser = argparse.ArgumentParser()
    parser.add_argument('-i', '--input', help='Input folder', required=True)
    parser.add_argument('-l', '--labels', help='Input lables', required=True)
    parser.add_argument('-o', '--output', help='Output folder', required=True)
    args = parser.parse_args()
    if not check_if_file_exists(args.labels):
        print 'Lable file not found!'
        exit()
    if not check_if_folder_exists(args.input):
        print 'Input folder not found!'
        exit()
    if not check_if_folder_exists(args.output):
        print 'Output folder not found!'
        print 'Going to create one.'
        os.makedirs(args.output)
    return args


ARGS = parse_args()


def main():
    labels_df = pd.read_csv(ARGS.labels)
    # Create dataframe matching to GTSRB dataset layout
    output_df = pd.DataFrame(columns=['Filename', 'Width', 'Height', 'Roi.x1',
                                      'Roi.y1', 'Roi.x2', 'Roi.y2', 'ClassID',
                                      'XOffset', 'YOffset', 'Rotation'])
    # Check if labels CSV is in the right format
    if not check_if_lables_df_is_correct(labels_df):
        print 'Labels CSV file fomat is inccorect!'
        exit()
    p = tqdm(total=len(labels_df), unit='images')

    for i, _ in labels_df.iterrows():
        crop_image_and_save(labels_df, i, output_df)
        p.update()
        output_df.to_csv(os.path.join(ARGS.output, 'labels.csv'), index=False)


if __name__ == "__main__":
    main()
