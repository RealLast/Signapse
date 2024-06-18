import argparse
import os
import sys
import cv2
import collections
import numpy as np
import tensorflow as tf
import copy
import time

NUM_CLASSES = 12


def load_detection_model(path_to_detection_model):
    detection_graph = tf.Graph()
    with detection_graph.as_default():
        od_graph_def = tf.GraphDef()
        with tf.gfile.GFile(path_to_detection_model, 'rb') as fid:
            serialized_graph = fid.read()
            od_graph_def.ParseFromString(serialized_graph)
            tf.import_graph_def(od_graph_def, name='')
    return detection_graph


def detect_signs(frame, sess):
    # Network trained on RGB but CV Image ist BGR
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
    output_dict = sess.run(tensor_dict, feed_dict={image_tensor: np.expand_dims(frame, 0)})

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
