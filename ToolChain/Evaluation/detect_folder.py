import os
import argparse
import detector
import cv2
import tensorflow as tf
import pandas as pd

THRESHOLD = 0.1
MAX_SIZE = 300


def main():
    args = parse_args()
    output_df = pd.DataFrame(columns=['filename', 'class', 'confidence',
                                      'xmin', 'ymin', 'xmax', 'ymax'])
    ckpt_path = "/home/tripton/Schreibtisch/softwareprojekt2018/NeuralNetwork/Detection/networks"
    for file_m in os.listdir(ckpt_path):
        if os.path.isdir(os.path.join(ckpt_path, file_m)):
            network = detector.load_detection_model(os.path.join(ckpt_path, file_m, "frozen_inference_graph.pb"))
            counter = 0
            with network.as_default():
                with tf.Session() as sess:
                    for filename in os.listdir(args.input):
                        # for filename in os.listdir(os.path.join(args.input, folder)):
                        if ".png" in filename:
                            # print(filename)
                            # detector.detect_signs(network)
                            image = load_image(os.path.join(args.input, filename))
                            dict_detector = detector.detect_signs(image, sess)
                            for i in range(0, dict_detector["num_detections"]):
                                confidence = dict_detector['detection_scores'][i]
                                # (im_height, im_width, _) = image.shape
                                im_height = 300
                                im_width = 300
                                x_min = int(dict_detector['detection_boxes'][i][1] * im_width)
                                x_max = int(dict_detector['detection_boxes'][i][3] * im_width)
                                y_min = int(dict_detector['detection_boxes'][i][0] * im_height)
                                y_max = int(dict_detector['detection_boxes'][i][2] * im_height)
                                detect_class = dict_detector['detection_classes'][i]
                                output_df.loc[counter] = [filename, detect_class, confidence, x_min, y_min, x_max, y_max]
                                counter += 1
                            output_df.to_csv(os.path.join(args.output, str(file_m) + 'labels.csv'), index=False)
                            # cv2.imshow("Image", image)


def load_image(image_path):
    # print(image_path)
    img = cv2.imread(image_path)
    width, height = img.shape[1::-1]

    scale_factor_x = MAX_SIZE/float(width)
    scale_factor_y = MAX_SIZE/float(height)
    resized_img = cv2.resize(img, (0, 0), fx=scale_factor_x,
                             fy=scale_factor_y, interpolation=cv2.INTER_CUBIC)
    b, g, r = cv2.split(resized_img)       # get b,g,r
    return cv2.merge([r, g, b])     # switch it to rgb


def parse_args():
    parser = argparse.ArgumentParser(
        description="Detect all frames in folder and save to csv")

    parser.add_argument("-i",
                        "--input",
                        help="Input Folder",
                        required=True)

    parser.add_argument("-o",
                        "--output",
                        help="Output CSV",
                        required=True)
    args = parser.parse_args()
    if not os.path.isdir(args.input):
        print 'Input folder not found.'
        exit()
    return args


if __name__ == "__main__":
    main()
