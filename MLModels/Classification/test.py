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


validation_data_path = '/media/sofi9432/ESD-USB/split_img/validation'
file_count_train = 30140
file_count_val = 3362
epochs = 10

batch_size = 128
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

validation_generator = test_datagen.flow_from_directory(
                                    validation_data_path,
                                    target_size=(image_size, image_size),
                                    batch_size=batch_size,
                                    shuffle=False,
                                    class_mode='categorical')

print(validation_generator.class_indices)
# loaded_model.load_weights("model.h5")
MobileNet.load_weights("weights-improvement-144-0.95.hdf5")
print("Loaded model from disk")

# evaluate loaded model on test data
MobileNet.compile(optimizer='rmsprop',
                  loss='categorical_crossentropy',
                  metrics=['accuracy'])

# score = MobileNet.evaluate_generator(validation_generator)
# print("%s: %.2f%%" % (MobileNet.metrics_names[1], score[1]*100))
print(len(validation_generator))
Y_probabilities = MobileNet.predict_generator(validation_generator)
y_pred = np.argmax(Y_probabilities, axis=1)
print(y_pred)
# y_pred = np.argmax(Y_probabilities, axis=1)
# print('predictet')
# y_true = np.array([0] * 1024 + [1] * 1024)
#
# y_pred = (Y_probabilities > 0.5)
# print(y_pred)
#
# cnfm = confusion_matrix(validation_generator.classes, y_pred)
# print('cnfm done')
# fig = plt.figure()
# plt.matshow(cnfm)
# plt.savefig('conf.png')
# print(cnfm)


# class_names = validation_generator.class_indices
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


def plot_confusion_matrix(cm, classes,
                          normalize=False,
                          title='Confusion matrix',
                          cmap=plt.cm.Blues):
    """
    This function prints and plots the confusion matrix.
    Normalization can be applied by setting `normalize=True`.
    """
    if normalize:
        cm = cm.astype('float') / cm.sum(axis=1)[:, np.newaxis]
        print("Normalized confusion matrix")
    else:
        print('Confusion matrix, without normalization')

    print(cm)

    plt.imshow(cm, interpolation='nearest', cmap=cmap)
    plt.title(title)
    plt.colorbar()
    tick_marks = np.arange(len(classes))
    plt.xticks(tick_marks, classes, rotation=45)
    plt.yticks(tick_marks, classes)

    # fmt = '.1f' if normalize else 'd'
    thresh = cm.max() / 2.
    # for i, j in itertools.product(range(cm.shape[0]), range(cm.shape[1])):
    # plt.text(j, i, format(cm[i, j]),
    #         horizontalalignment="center",
    #         color="white" if cm[i, j] > thresh else "black")

    plt.tight_layout()
    plt.ylabel('Ground Truth')
    plt.xlabel('Prediction')


# Compute confusion matrix
cnf_matrix = confusion_matrix(validation_generator.classes, y_pred)
np.set_printoptions(precision=11)

# Plot non-normalized confusion matrix
plt.figure(figsize=(25, 15))
plot_confusion_matrix(cnf_matrix, classes=class_names,
                      title='Confusion matrix, without normalization')

# Plot normalized confusion matrix
fig = plt.figure(figsize=(35, 20),)
plot_confusion_matrix(cnf_matrix, classes=class_names, normalize=True,
                      title='Normalized confusion matrix')
plt.grid(color='grey', linestyle='-', linewidth=1)
plt.show()
fig.savefig('temp.png', dpi=fig.dpi)
