from keras.models import Sequential
from keras.layers import Dense
from keras.models import model_from_json
import numpy as np
import os
from keras.preprocessing.image import ImageDataGenerator
from keras_applications import mobilenet
from sklearn.metrics import confusion_matrix
import tui_tensorflow_utils
import matplotlib.pyplot as plt
print(__doc__)

import itertools
import numpy as np
import matplotlib.pyplot as plt

from sklearn import svm, datasets
from sklearn.model_selection import train_test_split
from sklearn.metrics import confusion_matrix
from sklearn.model_selection import cross_val_predict

# load json and create model
# json_file = open('model.json', 'r')
# loaded_model_json = json_file.read()
# json_file.close()
# loaded_model = model_from_json(loaded_model_json)
# load weights into new model
relu6 = mobilenet.relu6


validation_data_path = '/datasets/swp2018/split_img/validation'
file_count_train = 30140
file_count_val = 3362
epochs = 10

batch_size = 196
image_size = 96
alpha_val = 0.35

use_pretrained_weights = False

if use_pretrained_weights:
    weights_name = 'imagenet'
else:
    weights_name = None


def preprocess_input(x):
    x /= 127.5
    x -= 1.
    return x.astype(np.float32)


MobileNet = mobilenet.MobileNet(input_shape=(image_size, image_size, 3),
                                alpha=alpha_val,
                                depth_multiplier=1,
                                include_top=True,
                                weights=weights_name,
                                input_tensor=None,
                                pooling=None,
                                classes=47)

test_datagen = ImageDataGenerator(preprocessing_function=preprocess_input)

class_names = ['Board',
               'Misc Blue',
               'Misc RB',
               'Misc RW',
               'Misc White',
               'Misc Yellow',
               'Misc brown',
               'Misc green',
               'VZ 1000',
               'VZ 101',
               'VZ 101-11',
               'VZ 103-10',
               'VZ 103-20',
               'VZ 105-10',
               'VZ 105-20',
               'VZ 1053-35',
               'VZ 120',
               'VZ 121-10',
               'VZ 121-20',
               'VZ 123',
               'VZ 131',
               'VZ 133-10',
               'VZ 136-10',
               'VZ 274-10',
               'VZ 274-100',
               'VZ 274-120',
               'VZ 274-130',
               'VZ 274-20',
               'VZ 274-30',
               'VZ 274-40',
               'VZ 274-50',
               'VZ 274-60',
               'VZ 274-70',
               'VZ 274-80',
               'VZ 274.1',
               'VZ 274.2',
               'VZ 278',
               'VZ 282',
               'VZ 306',
               'VZ 310',
               'VZ 311',
               'VZ 325.1',
               'VZ 325.2',
               'VZ 330.1',
               'VZ 330.2',
               'VZ 331.1',
               'VZ 331.2']


values = np.zeros(201)
for i in range(144, 145):
    validation_generator = test_datagen.flow_from_directory(
        validation_data_path,
        target_size=(image_size, image_size),
        batch_size=batch_size,
        shuffle=False,
        class_mode='categorical')

    # loaded_model.load_weights("model.h5")MobileNet.load_weights("/localdat0/sofi9432/weights/weights-improvement-249-0.95.hdf5")
    print("Loaded model " + str(i) + " from disk")
    path = "/localdat0/sofi9432/weights/"
    files = []
    for k in os.listdir(path):
        if os.path.isfile(os.path.join(path, k)) and ('weights-improvement-' + str(i)) in k:
            files.append(k)
    MobileNet.load_weights(path + files[0])
    # evaluate loaded model on test data
    MobileNet.compile(optimizer='rmsprop',
                      loss='categorical_crossentropy',
                      metrics=['accuracy'])

    # score = MobileNet.evaluate_generator(validation_generator)
    # print("%s: %.2f%%" % (MobileNet.metrics_names[1], score[1]*100))

    Y_probabilities = MobileNet.predict_generator(validation_generator)
    y_pred = np.argmax(Y_probabilities, axis=1)
    # Compute confusion matrix
    cm = confusion_matrix(validation_generator.classes, y_pred)
    cm = cm.astype('float') / cm.sum(axis=1)[:, np.newaxis]
    sum_dig = 0.0
    for j in range(0, len(cm)):
        sum_dig = sum_dig + cm[j][j]
    values[i] = sum_dig
    print(sum_dig)
print(np.argmax(values))
print(values)
