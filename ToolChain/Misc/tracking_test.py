###############################################################################
# usage:   WIP: Test for combination of Detection and Tracking      .              #
# author:  Söhnke Benedikt Fischedick                                         #
# created: 15.05.2018                                                         #
###############################################################################

import argparse
import os
import sys
import cv2
import collections
import numpy as np
import tensorflow as tf
import copy
import time

NUM_CLASSES = 15
PATH_TO_DETECTION_MODEL = 'detection_network.pb'


def main():
    args = parse_args()
    video = cv2.VideoCapture(args.input)
    ring_buffer = collections.deque(maxlen=20)
    detection_graph = load_detection_model()
    detect_frame_rate = int(args.framerate)
    last_sign_num = 0
    old_bbox = (0, 0, 0, 0)
    tracked = False
    # Ext if video not opened.
    if not video.isOpened():
        print "Could not open video"
        sys.exit()
    frame_count = 1
    while True:
        # Read a new frame
        ok, frame = video.read()

        if not ok:
            break
        # Append frame to ring_buffer
        ring_buffer.append(frame)
        if frame_count % (detect_frame_rate) == 0:
            output_dict = detect_signs(ring_buffer[len(ring_buffer)-1], detection_graph)
            num_signs, bbox, cur_frame = draw_detect_rects(ring_buffer[len(ring_buffer)-1], output_dict)
            ring_buffer[len(ring_buffer)-1] = cur_frame
            frame_count = 0
            frame_to_show = ring_buffer[9]
            if num_signs == 0 and last_sign_num > 0:
                # Track last elements
                print 'Track'
                tracked = True
                track_sign(old_bbox, ring_buffer)
                # frame_to_show = ring_buffer[8]
                # track_sign(old_bbox, pastframe_ring_buffer)
                # for i in range (2,-1, -1):
                # cv2.imshow("Tracking", old_ring_buffer[i])
            # Newest frame bbox and num_signs is now the old value
            last_sign_num = num_signs
            old_bbox = bbox
            # Show oldest element where sign detection was
            if tracked:
                for i in range(6, 10):
                    cv2.imshow("TSD", ring_buffer[i])
                    time.sleep(2)

            else:
                cv2.imshow("TSD", ring_buffer[9])
        frame_count += 1
        # Exit if ESC pressed
        k = cv2.waitKey(1) & 0xff
        if k == 27:
            break


def load_detection_model():
    detection_graph = tf.Graph()
    with detection_graph.as_default():
        od_graph_def = tf.GraphDef()
        with tf.gfile.GFile(PATH_TO_DETECTION_MODEL, 'rb') as fid:
            serialized_graph = fid.read()
            od_graph_def.ParseFromString(serialized_graph)
            tf.import_graph_def(od_graph_def, name='')
    return detection_graph


def detect_signs(frame, graph):
    # Network trained on RGB but CV Image ist BGR
    image = cv2.cvtColor(frame, cv2.COLOR_BGR2RGB)
    image = cv2.resize(image, (300, 300), cv2.INTER_CUBIC)
    with graph.as_default():
        with tf.Session() as sess:
            # Get handles to input and output tensors
            ops = tf.get_default_graph().get_operations()
            all_tensor_names = {output.name for op in ops for output in op.outputs}
            tensor_dict = {}
            for key in [
                'num_detections', 'detection_boxes', 'detection_scores',
                'detection_classes'
            ]:
                tensor_name = key + ':0'
                if tensor_name in all_tensor_names:
                    tensor_dict[key] = tf.get_default_graph().get_tensor_by_name(
                        tensor_name)

            image_tensor = tf.get_default_graph().get_tensor_by_name('image_tensor:0')

            # Run inference
            output_dict = sess.run(tensor_dict,
                                   feed_dict={image_tensor: np.expand_dims(image, 0)})

            # all outputs are float32 numpy arrays, so convert types as appropriate
            output_dict['num_detections'] = int(output_dict['num_detections'][0])
            output_dict['detection_classes'] = output_dict['detection_classes'][0].astype(np.uint8)
            output_dict['detection_boxes'] = output_dict['detection_boxes'][0]
            output_dict['detection_scores'] = output_dict['detection_scores'][0]
        return output_dict


def draw_detect_rects(frame, dict):
    (im_height, im_width, _) = frame.shape
    num_of_signs = 0
    bbox = (0, 0, 0, 0)
    for i in range(0, dict['num_detections']):
        confidence = dict['detection_scores'][i]
        # 0.75 is threshold for confidence value of detection network
        if confidence > 0.75:
            detct_class = dict['detection_classes'][i]
            # get offsets for rect/bbox
            x_left = int(dict['detection_boxes'][i][1] * im_width)
            x_right = int(dict['detection_boxes'][i][3] * im_width)
            y_left = int(dict['detection_boxes'][i][0] * im_height)
            y_right = int(dict['detection_boxes'][i][2] * im_height)
            # only save newest bbox for simplicity
            bbox = (x_left, y_left, x_right, y_right)
            # Draw rect into image and add text of class and confidence
            cv2.putText(frame, str(detct_class) + ': ' + str(confidence), (x_left-20, y_left-20), cv2.FONT_HERSHEY_SIMPLEX, 0.75, (255, 0, 0), 2)
            cv2.rectangle(frame, (x_left, y_left), (x_right, y_right), (255, 0, 0), 3)
            num_of_signs += 1
    return num_of_signs, bbox, frame


def track_sign(bbox, pastframe):
    tracker = cv2.TrackerMedianFlow_create()
    p1 = (int(bbox[0]), int(bbox[1]))
    p2 = (int(bbox[2]), int(bbox[3]))
    # cv2.rectangle(pastframe[9], p1, p2, (0,0,255), 4)
    # bbox = cv2.selectROI(ring_buffer[len(ring_buffer-2)], False)
    ok = tracker.init(pastframe[9], bbox)
    # print str(bbox[0]) +  ',' + str(bbox[1]) +  ',' + str(bbox[2]) + ',' + str(bbox[3])
    for i in range(8, 5, -1):
        # Update tracker
        print i
        ok, bbox = tracker.update(pastframe[i])

        if ok:
            # Eigentlich Klassifizieren
            # Tracking success
            p1 = (int(bbox[0]), int(bbox[1]))
            p2 = (int(bbox[0] + bbox[2]), int(bbox[1] + bbox[3]))
            cv2.rectangle(pastframe[i], p1, p2, (0, 0, 255), 4)
            cv2.imshow('Neu', pastframe[i])
            print 'Succ'
        else:
            print 'failed'


def parse_args():
    parser = argparse.ArgumentParser(
        description="Test logic and neural network")

    parser.add_argument("-i",
                        "--input",
                        help="Input Video",
                        required=True)

    parser.add_argument("-f",
                        "--framerate",
                        help="Set framerate from video, which will be feed through pipeline",
                        required=True)
    args = parser.parse_args()
    if not os.path.isfile(args.input):
        print 'Input file not found.'
        exit()
    return args


if __name__ == "__main__":
    main()
