from __future__ import absolute_import
from __future__ import division
from __future__ import print_function

import tui_tensorflow_utils
import keras
import math
import numpy as np
import os
from keras.preprocessing.image import ImageDataGenerator
from keras_applications import mobilenet
from keras.optimizers import SGD
from sklearn.utils import class_weight

train_data_path = '/datasets/swp2018/split_img/train'
validation_data_path = '/datasets/swp2018/split_img/validation'
file_count_train = 134443
epochs = 200


image_size = 96
alpha_val = 0.35
batch_size = 128
learn_rate = 0.045
decay = 0.00004
momentum = 0.9

# To-Do: get num from generator
classes = 47

use_pretrained_weights = False

# Get filecount in trainig folder
# file_count_train = len([name for name in os.listdir(train_data_path) if os.path.isfile(os.path.join(train_data_path, name))])
file_count_val = 30517
# file_count_val = len([name for name in os.listdir(validation_data_path) if os.path.isfile(os.path.join(validation_data_path, name))])
# print(file_count_train)
# Input of network is between -1 to 1 so we need to preprocess every RGB Pixel


def preprocess_input(x):
    x /= 127.5
    x -= 1.
    return x.astype(np.float32)


def save_weights():
    # serialize weights to HDF5
    MobileNet.save_weights("model.h5")
    print("Saved model to disk")


# Load pretrained weights if wantet
if use_pretrained_weights:
    weights_name = 'imagenet'
else:
    weights_name = None


#
train_datagen = ImageDataGenerator(
        rotation_range=15,
        width_shift_range=0.15,
        height_shift_range=0.15,
        shear_range=0.15,
        zoom_range=0.3,
        horizontal_flip=False,
        preprocessing_function=preprocess_input)

test_datagen = ImageDataGenerator(preprocessing_function=preprocess_input)

train_generator = train_datagen.flow_from_directory(
        train_data_path,
        target_size=(image_size, image_size),
        batch_size=batch_size,
        class_mode='categorical')

validation_generator = test_datagen.flow_from_directory(
        validation_data_path,
        target_size=(image_size, image_size),
        batch_size=batch_size,
        class_mode='categorical')


# Load custom Relu6 layer for MobileNetv2
relu6 = mobilenet.relu6

# Load network
MobileNet = mobilenet.MobileNet(input_shape=(image_size, image_size, 3),
                                alpha=alpha_val,
                                depth_multiplier=1,
                                include_top=True,
                                weights=weights_name,
                                input_tensor=None,
                                pooling=None,
                                classes=classes)


# generate class weights
class_weights = class_weight.compute_class_weight('balanced', np.unique(train_generator.classes), train_generator.classes)

# Use callback to save weights after every epoch
filepath = "/localdat0/sofi9432/weights/weights-improvement-{epoch:02d}-{val_acc:.2f}.hdf5"
checkpoint = keras.callbacks.ModelCheckpoint(filepath, monitor='val_loss', verbose=0, save_best_only=False, save_weights_only=True, mode='auto', period=1)
callbacks_list = [checkpoint]

# Set sgd parameters
sgd = SGD(lr=learn_rate, decay=decay, momentum=momentum, nesterov=True)

MobileNet.compile(optimizer=sgd,
                  loss='categorical_crossentropy',
                  metrics=['accuracy'])

# Start training
MobileNet.fit_generator(train_generator,
                        steps_per_epoch=math.ceil(file_count_train/batch_size),
                        epochs=epochs,
                        validation_data=validation_generator,
                        validation_steps=math.ceil(file_count_val/batch_size),
                        class_weight=class_weights,
                        callbacks=callbacks_list)
