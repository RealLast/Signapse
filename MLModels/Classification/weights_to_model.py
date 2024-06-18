import keras
from keras import backend as K
from keras.models import load_model
from tensorflow.python.saved_model import builder as saved_model_builder
from tensorflow.python.saved_model.signature_def_utils import predict_signature_def
from tensorflow.python.saved_model import tag_constants
from keras_applications import mobilenet
from keras.optimizers import SGD
import tui_tensorflow_utils

MobileNet = mobilenet.MobileNet(input_shape=(96, 96, 3),
                                alpha=0.35,
                                depth_multiplier=1,
                                include_top=True,
                                weights=None,
                                input_tensor=None,
                                pooling=None,
                                classes=47)
# Load the Keras model
# keras_model = load_model(path_to_h5)
MobileNet.load_weights("/localdat0/sofi9432/weights/weights-improvement-249-0.95.hdf5")
sgd = SGD(lr=0.045, decay=0.00004, momentum=0.9, nesterov=True)
MobileNet.compile(optimizer=sgd, loss='categorical_crossentropy', metrics=['accuracy'])

MobileNet.save("model.h5")
