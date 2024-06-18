import argparse
parser = argparse.ArgumentParser(description='set input arguments')
parser.add_argument('-input_model_file', action="store",
                    dest='input_model_file', type=str, default='model.h5')
parser.add_argument('-output_model_file', action="store",
                    dest='output_model_file', type=str, default='')
parser.add_argument('-output_node_prefix', action="store",
                    dest='output_node_prefix', type=str, default='output_node')

parser.add_argument('-f')
args = parser.parse_args()
parser.print_help()
print('input args: ', args)

import keras
from keras.models import load_model
import tensorflow as tf
from pathlib import Path
from keras import backend as K
from keras_applications import mobilenet
from tensorflow.python.keras._impl.keras.utils.generic_utils import CustomObjectScope

from keras.utils.generic_utils import get_custom_objects

from tensorflow.python.framework import graph_util
from tensorflow.python.framework import graph_io


output_fld = args.input_fld if args.output_fld == '' else args.output_fld
if args.output_model_file == '':
    args.output_model_file = str(Path(args.input_model_file).name) + '.pb'
weight_file_path = str(Path(args.input_fld) / args.input_model_file)


K.set_learning_phase(0)


# with CustomObjectScope({'relu6': relu6_custom}):
net_model = mobilenet.MobileNet(input_shape=(96, 96, 3),
                                alpha=0.35,
                                depth_multiplier=1,
                                include_top=True,
                                weights=None,
                                input_tensor=None,
                                pooling=None,
                                classes=47)

net_model.load_weights(weight_file_path)

num_output = args.num_outputs
pred = [None]*num_output
pred_node_names = [None]*num_output
for i in range(num_output):
    pred_node_names[i] = args.output_node_prefix+str(i)
    pred[i] = tf.identity(net_model.outputs[i], name=pred_node_names[i])
print('output nodes names are: ', pred_node_names)

constant_graph = graph_util.convert_variables_to_constants(sess, sess.graph.as_graph_def(), pred_node_names)
graph_io.write_graph(constant_graph, output_fld, args.output_model_file, as_text=False)
print('saved the freezed graph (ready for inference) at: ', str(Path(output_fld) / args.output_model_file))
