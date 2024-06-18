""" Script to extract frames from video
"""
import os
import argparse
import sys
import cv2
import pandas as pd


# Only used to show a progressbar
def progress(count, total, suffix=''):
    """ Progressbar
    """
    bar_len = 60
    filled_len = int(round(bar_len * count / float(total)))
    percents = round(100.0 * count / float(total), 1)
    bar_str = '=' * filled_len + '-' * (bar_len - filled_len)
    sys.stdout.write('[%s] %s%s ...%s\r' % (bar_str, percents, '%', suffix))
    sys.stdout.flush()


# Creates a 'delete.csv'
def create_csv(args):
    """ create delete.csv for later usage with delete_from_csv.py
    """
    raw_data = {'from': [0], 'to': [0]}
    df = pd.DataFrame(raw_data, columns=['from', 'to'])
    df.to_csv(args.output + '/delete.csv', index=False)


# Checks if -o folder exists. If not it will create it. After that the frames
# will be extractet from the Video and delete.csv will be created.
def check_io(args):
    """ Check if args a correct
    """
    if not os.path.isfile(args.input):
        print 'File ' + args.input + ' doesn\'t exists!'
    else:
        print 'Input video: ' + args.input
    if not os.path.isdir(args.output):
        print 'Folder ' + args.output + ' not found. Going to create one!'
        os.makedirs(args.output)
    print 'Output folder: ' + args.output
    video_to_frames(args)
    create_csv(args)


# Will extreact every frame from video to .png image
def video_to_frames(args):
    """ Extract frames from video
    """
    vidcap = cv2.VideoCapture(args.input)
    success, image = vidcap.read()
    length = int(vidcap.get(cv2.CAP_PROP_FRAME_COUNT))
    count = 0
    success = True
    while success:
        cv2.imwrite(args.output + "/frame%d.png" % count, image,
                    [cv2.IMWRITE_PNG_COMPRESSION, 0])
        success, image = vidcap.read()
        count += 1
        progress(count, length, '')
    print '\nSuccess!'


# Only used for argument parsing. after that check_io will get called
def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('-i', '--input', help='Input file', required=True)
    parser.add_argument('-o', '--output', dest='output',
                        help='Output folder', required=True)
    args = parser.parse_args()
    check_io(args)


if __name__ == "__main__":
    main()
