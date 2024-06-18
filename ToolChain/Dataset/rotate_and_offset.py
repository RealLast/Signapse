###############################################################################
# usage:   Replaces information in files using a CSV dictionary.              #
# author:  Robert Niklas Bock and Soehnke Benedikt Fischedick                 #
# created: 15.05.2018                                                         #
###############################################################################
""" Rotates image, adds offset and cut in border
"""
import argparse
import os
import random
import cv2
import numpy as np
from scipy import ndimage
from tqdm import tqdm
import pandas as pd

from ensuring import ensure

###############################################################################
# PARSER

PARSER = argparse.ArgumentParser(
    description="Replaces information in CSV-files using a "
                "CSV dictionary")

PARSER.add_argument("-i",
                    "--input",
                    help="Folder with source images",
                    required=True)

PARSER.add_argument("-l",
                    "--labels",
                    help="path to csv with label data",
                    required=True)

PARSER.add_argument("-d",
                    help="draws debug information on samples",
                    action='store_true')

PARSER.add_argument("-o",
                    "--output",
                    help="Folder with source images",
                    required=True)

ARGS = PARSER.parse_args()


###############################################################################
# CLASSES

class Rect(object):
    """ Describes a rectangle
    """

    def __init__(self, x1, y1, x2, y2):
        ensure(round_i(x2) > round_i(x1),
               "x2 has to be bigger than x1; "
               "{} !> {}".format(round_i(x2), round_i(x1)))
        ensure(round_i(y2) > round_i(y1),
               "y2 has to be bigger than y1; "
               "{} !> {}".format(round_i(y2), round_i(y1)))

        self.x1 = round_i(x1)
        self.y1 = round_i(y1)
        self.x2 = round_i(x2)
        self.y2 = round_i(y2)

    def get_width(self):
        """ retuns width
        """
        return self.x2 - self.x1

    def get_height(self):
        """ returns height
        """
        return self.y2 - self.y1

    def get_diagonal(self):
        """ returns diagonal
        """
        return euclidean_distance((self.x1, self.y1), (self.x2, self.y2))

    def get_center(self):
        """ returns center point
        """
        return (round_i(self.get_width() / 2), round_i(self.get_height() / 2))

    def center_on_point(self, point):
        """ To-Do
        """
        width_ = round_i(self.get_width() / 2)
        height_ = round_i(self.get_height() / 2)

        self.x1 = point[0] - width_
        self.y1 = point[1] - height_
        self.x2 = point[0] + width_
        self.y2 = point[1] + height_

    def translate(self, vector):
        """ To-Do
        """
        self.x1 += vector[0]
        self.x2 += vector[0]
        self.y1 += vector[1]
        self.y2 += vector[1]

    def get_tl(self):
        """ return top left border point
        """
        return (self.x1, self.y1)

    def get_tr(self):
        """ return top right border point
        """
        return (self.x2, self.y1)

    def get_bl(self):
        """ return bottom left border point
        """
        return (self.x1, self.y2)

    def get_br(self):
        """ return bottom right border point
        """
        return (self.x2, self.y2)

    def as_tuple(self):
        """ return tupel
        """
        return (self.x1, self.y1, self.x2, self.y2)


###############################################################################
# FUNCTIONS

def round_i(n):
    """ round and cast to itneger
    """
    return int(round(n))


def euclidean_distance(point_a, point_b):
    """ returns euclidean_distance
    """
    dx = point_a[0] - point_b[0]
    dy = point_a[1] - point_b[1]

    return np.sqrt(dx ** 2 + dy ** 2)


def get_new_bounding_box(*points):
    """ calculates new bounding box
    """
    x_values = []
    y_values = []

    for p in points:
        x_values.append(p[0])
        y_values.append(p[1])

    return Rect(min(x_values),
                min(y_values),
                max(x_values),
                max(y_values))


def translate_point(point, vector):
    """ moves a point along a vector
    """
    return (point[0] + vector[0], point[1] + vector[1])


def rotate_point(point, origin, angle):
    """
    Rotate a point counterclockwise by a given angle around a given origin.

    The angle should be given in radians.
    """

    ox, oy = origin
    px, py = point
    angle = np.radians(angle)

    qx = round_i(ox + np.cos(angle) * (px - ox) - np.sin(angle) * (py - oy))
    qy = round_i(oy + np.sin(angle) * (px - ox) + np.cos(angle) * (py - oy))
    return (qx, qy)


def get_aoi(bounds, border_abs, border_rel, max_rel_offset):
    """ finds the maximum area that could be intersting to cut out a picture
    """
    max_border = max(bounds.get_diagonal() * border_rel, border_abs)

    abs_max_offset = round_i(bounds.get_diagonal() * max_rel_offset)

    # maximum rotational extend
    max_rot_ext = round_i(bounds.get_diagonal() + 2 * max_border + _
                          2 * abs_max_offset + 4)

    # center of sign
    sgn_cntr = translate_point(bounds.get_center(), bounds.get_tl())

    aoi = Rect(sgn_cntr[0] - round_i(max_rot_ext / 2),
               sgn_cntr[1] - round_i(max_rot_ext / 2),
               sgn_cntr[0] + round_i(max_rot_ext / 2),
               sgn_cntr[1] + round_i(max_rot_ext / 2))

    return aoi


def cut_with_zero_padding(img, bounds):
    """ cuts a part of an image and adds black borders if necessary
    """
    canvas = np.zeros((bounds.get_height(),
                       bounds.get_width(),
                       3), dtype=np.uint8)

    c_width, c_height = canvas.shape[1::-1]
    img_width, img_height = img.shape[1::-1]

    for x in xrange(0, c_width):
        for y in xrange(0, c_height):
            if (bounds.x1 + x >= 0 and bounds.x1 + x < img_width and _
                    bounds.y1 + y >= 0 and bounds.y1 + y < img_height):

                canvas[y, x] = img[bounds.y1 + y, bounds.x1 + x]

    return canvas


def get_randomized_area(img, bounds, max_angle):
    """ gets an area from an image which is rotated by a random angle
    """
    img_width, img_height = img.shape[1::-1]
    img_center = (img_width / 2, img_height / 2)

    bounds.center_on_point(img_center)

    actual_angle = random.randint(-max_angle, max_angle)

    rot_img = ndimage.rotate(img, actual_angle)
    rot_img_width, rot_img_height = rot_img.shape[1::-1]
    rot_img_center = (round_i(rot_img_width / 2), round_i(rot_img_height / 2))

    diff_rot_center = (rot_img_center[0] - img_center[0],
                       rot_img_center[1] - img_center[1])

    rot_tl = rotate_point(bounds.get_tl(), img_center, -actual_angle)
    rot_tr = rotate_point(bounds.get_tr(), img_center, -actual_angle)
    rot_bl = rotate_point(bounds.get_bl(), img_center, -actual_angle)
    rot_br = rotate_point(bounds.get_br(), img_center, -actual_angle)

    trans_tl = translate_point(rot_tl, diff_rot_center)
    trans_tr = translate_point(rot_tr, diff_rot_center)
    trans_bl = translate_point(rot_bl, diff_rot_center)
    trans_br = translate_point(rot_br, diff_rot_center)

    rotate_bounds = get_new_bounding_box(trans_tl,
                                         trans_tr,
                                         trans_bl,
                                         trans_br)

    return (rotate_bounds, actual_angle, rot_img)


def buffer_area(rotate_bounds, border_abs, border_rel):
    """ adds borders to an area
    """
    x_border_size = max(rotate_bounds.get_width() * border_rel, border_abs)
    y_border_size = max(rotate_bounds.get_height() * border_rel, border_abs)

    buffered_bounds = Rect(rotate_bounds.x1 - x_border_size,
                           rotate_bounds.y1 - y_border_size,
                           rotate_bounds.x2 + x_border_size,
                           rotate_bounds.y2 + y_border_size)

    return buffered_bounds


def offset_area(buffered_bounds, max_rel_offset):
    """ offsets an area
    """
    abs_max_offset = round_i(buffered_bounds.get_width() * max_rel_offset)
    actual_offset_x = random.randint(-abs_max_offset, abs_max_offset)
    actual_offset_y = random.randint(-abs_max_offset, abs_max_offset)

    offset_bounds = Rect(buffered_bounds.x1 + actual_offset_x,
                         buffered_bounds.y1 + actual_offset_y,
                         buffered_bounds.x2 + actual_offset_x,
                         buffered_bounds.y2 + actual_offset_y)

    return (offset_bounds, actual_offset_x, actual_offset_y)


def get_randomized_batch_of_img(img, count, bounds, border_abs,
                                border_rel, max_rel_offset, max_angle):
    """ generates multiple randomly rotated and moved images
    """
    aoi = get_aoi(bounds, border_abs, border_rel, max_rel_offset)
    small_img = cut_with_zero_padding(img, aoi)

    batch = []

    for _ in xrange(count):
        rotated_bounds, angle, rot_img = get_randomized_area(small_img,
                                                             bounds,
                                                             max_angle)

        buffered_bounds = buffer_area(rotated_bounds, border_abs, border_rel)
        offset_bounds, off_x, off_y = offset_area(buffered_bounds,
                                                  max_rel_offset)

        sign = cut_with_zero_padding(rot_img, offset_bounds)
        batch.append((sign, rotated_bounds, angle, off_x, off_y))

    return batch


def cut_image(img, bounds, border_abs, border_rel, max_rel_offset, max_angle):
    """ Cuts sign from image + border
    """
    # Get image size for setting correct border
    img_width, img_height = img.shape[1::-1]
    img_center = (img_width / 2, img_height / 2)

    x_border_size = max(bounds.get_width() * border_rel, border_abs)
    y_border_size = max(bounds.get_height() * border_rel, border_abs)

    abs_max_offset = round_i(bounds.get_width() * max_rel_offset)

    actual_offset_x = random.randint(-abs_max_offset, abs_max_offset)
    actual_offset_y = random.randint(-abs_max_offset, abs_max_offset)
    actual_angle = random.randint(-max_angle, max_angle)

    rot_img = ndimage.rotate(img, actual_angle)

    rot_img_width, rot_img_height = rot_img.shape[1::-1]
    rot_img_center = (rot_img_width / 2, rot_img_height / 2)

    diff_rot_center = (rot_img_center[0] - img_center[0],
                       rot_img_center[1] - img_center[1])

    rot_tl = rotate_point(bounds.get_tl(), img_center, -actual_angle)
    rot_tr = rotate_point(bounds.get_tr(), img_center, -actual_angle)
    rot_bl = rotate_point(bounds.get_bl(), img_center, -actual_angle)
    rot_br = rotate_point(bounds.get_br(), img_center, -actual_angle)

    trans_tl = translate_point(rot_tl, diff_rot_center)
    trans_tr = translate_point(rot_tr, diff_rot_center)
    trans_bl = translate_point(rot_bl, diff_rot_center)
    trans_br = translate_point(rot_br, diff_rot_center)

    rotate_bounds = get_new_bounding_box(trans_tl, trans_tr,
                                         trans_bl, trans_br)

    new_bounds = Rect(rotate_bounds.x1 - x_border_size + actual_offset_x,
                      rotate_bounds.y1 - y_border_size + actual_offset_y,
                      rotate_bounds.x2 + x_border_size + actual_offset_x,
                      rotate_bounds.y2 + y_border_size + actual_offset_y)

    canvas = np.zeros((new_bounds.get_height(),
                       new_bounds.get_width(),
                       3), dtype=np.uint8)

    for x in xrange(0, canvas.shape[1]):
        # cv2.imwrite(os.path.normpath('D:\Robert\Desktop\pinguin-crop.png'),
        # canvas, [cv2.IMWRITE_PNG_COMPRESSION, 0])
        for y in xrange(0, canvas.shape[0]):
            if (new_bounds.x1 + x >= 0 and _
                    new_bounds.x1 + x < rot_img_width and _
                    new_bounds.y1 + y >= 0 and _
                    new_bounds.y1 + y < rot_img_height):

                canvas[y, x] = rot_img[new_bounds.y1 + y, new_bounds.x1 + x]

    return (canvas, actual_offset_x, actual_offset_y, actual_angle,
            x_border_size, y_border_size, rot_img)


def crop_and_save2(labels_df, output_df, index, count,
                   border_abs, border_rel, max_rel_offset, max_rotation):
    """ new version of crop and save, saves all subimages
    """
    file_path = os.path.join(ARGS.input, labels_df['filename'][index])
    img = cv2.imread(file_path)

    xmin = labels_df['xmin'][index]
    xmax = labels_df['xmax'][index]
    ymin = labels_df['ymin'][index]
    ymax = labels_df['ymax'][index]

    r = Rect(xmin, ymin, xmax, ymax)

    batch = get_randomized_batch_of_img(img, count, r, border_abs, border_rel,
                                        max_rel_offset, max_rotation)

    for i in xrange(len(batch)):
        sign, rotated_bounds, angle, off_x, off_y = batch[i]
        img_width, img_height = sign.shape[1::-1]
        img_center = (round_i(img_width / 2), round_i(img_height / 2))
        rotated_bounds.center_on_point(img_center)
        rotated_bounds.translate((-off_x, -off_y))
        x1, y1, x2, y2 = rotated_bounds.as_tuple()

        if ARGS.d:
            cv2.rectangle(sign, (x1, y1), (x2, y2), (0, 0, 255), 1)

        filename = (str(index) + "_" + _
                    str(i) + "_" + _
                    labels_df['filename'][index])
        cv2.imwrite(os.path.normpath(
            os.path.join(ARGS.output, filename)),
                    sign,
                    [cv2.IMWRITE_PNG_COMPRESSION, 0])

        sub_folder_path = os.path.normpath(
            os.path.join(ARGS.output, labels_df['class'][index]))

        if not os.path.isdir(sub_folder_path):
            os.makedirs(sub_folder_path)

        cv2.imwrite(os.path.normpath(
            os.path.join(sub_folder_path, filename)),
                    sign,
                    [cv2.IMWRITE_PNG_COMPRESSION, 0])

        output_df.loc[index * count + i] = [filename, img_width,
                                            img_height, x1,
                                            y1, x2,
                                            y2, labels_df['class'][index],
                                            off_x, off_y,
                                            angle]


###############################################################################
# MAIN


ABS_BORDER = 5  # px
REL_BORDER = 0.1  # * 100 %
MAX_REL_OFFSET = 0.15  # * 100 %
MAX_ANGLE = 5  # degree
ITERATIONS_PER_IMAGE = 8


def main():
    input_df = pd.read_csv(ARGS.labels)
    output_df = pd.DataFrame(columns=['Filename', 'Width', 'Height',
                                      'Roi.x1', 'Roi.y1', 'Roi.x2',
                                      'Roi.y2', 'ClassID', 'XOffset',
                                      'YOffset', 'Rotation'])

    p = tqdm(total=len(input_df) * ITERATIONS_PER_IMAGE, unit='images')

    for i, _ in input_df.iterrows():
        crop_and_save2(input_df, output_df, i, ITERATIONS_PER_IMAGE,
                       ABS_BORDER, REL_BORDER, MAX_REL_OFFSET, MAX_ANGLE)

        p.update(ITERATIONS_PER_IMAGE)

        if i % 32 == 0:
            output_df.to_csv(os.path.join(ARGS.output, 'labels_plus.csv'),
                             index=False)

    output_df.to_csv(os.path.join(ARGS.output, 'labels_plus.csv'), index=False)


###############################################################################
# MAIN CATCH

if __name__ == "__main__":
    main()
