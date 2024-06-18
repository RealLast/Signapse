# Neural Network aus der 1. Phase

In der Einarbeitungszeit wurde zum Testen von Detektion Netzwerken, ein[vortrainiertes Modell von Google](https: // github.com/tensorflow/models/blob/master/research/object_detection/g3doc/detection_model_zoo.md) benutzt.

Diese ist ein Single Shot MultiBox Detector Netzwerk in Kombination mit dem MobileNetv2 Netzwerk, welches auf den[COCO Datensatz](http: // cocodataset.org /  # home) vortrainiert ist.

Dieses wurde danach auf den[GTSDB Datensatz](http: // benchmark.ini.rub.de /?section=gtsdb & subsection=news) nachtrainiert.

Um dies zutun haben wir die[Tensorflow Object Detection API](https: // github.com/tensorflow/models/tree/master/research/object_detection) verwendet.

Dazu mussten wir die Datein vom GTSDB in ein kompatibles Format konvertieren.
Hierzu mussten die Labels konvertiert werden(siehe Toolchain/Dataset/gtsdb_to_csv.py) und die Bilder in ein kompatibles Dateiformat geändert werden(in unserem Fall wurde png gewählt).

Anschließend konnte diese in das Tensorflow Format '.record' konvertiert werden. Dazu haben wir den vorhandenen Datensatz geteilt.
Ca. 90 % wurden zum Trainieren benutz. Der Rest zum Testen.

Danach konnte das Netzwerk mittels Tensorflow und Python auf unser Problem nachtrainiert werden.

# Neural Network aus der 2. Phase
Da als Detektions Netzwerk SSDLite verwendet wird, und es in der[Tensorflow Object Detection API](https: // github.com/tensorflow/models/tree/master/research/object_detection) bereits eine Implementierung gibt, wurde diese als Grundlage genommen.
Das die bessagte API aber weitaus mehr Detektions Netzwerke unterstützt, wurde der Original Source Code gecloned und soweit reduziert, dass er lediglich mit SSDLite funktioniert.

Zu installation muss folgendes gemacht werde:
1. conda install protobuf
2. sudo apt-get install python-pil python-lxml python-tk
3. pip install - r requierments.txt

# From softwareprojekt2018/NeuralNetwork/Detection
4. Compile protobuf libary and add Libaries to Pythonpath
protoc object_detection/protos/*.proto - -python_out=.
export PYTHONPATH=$PYTHONPATH: `pwd`: `pwd`/slim

5. Check if installation was corrcect
python object_detection/builders/model_builder_test.py

Einstellung der Lernparamter:

Zum einstellen der gewünschten Lernparamter, muss in dem Verzeichnis softwareprojekt2018/NeuralNetwork/Detection/object-detection/SSDLite die pipeline.config verändert werden.

Umwandeln des Datensatz in tfrecord:

Da die API ein eigenes Format für das Training verwendet, muss der bestehende Daten sazu noch umgewandelt werdem.
Dazu muss das Skript softwareprojekt2018/NeuralNetwork/Detection/object-detection/utils/generate_tfrecord.py verwendet werden.

Dazu müssen folgende Parameter angeben werden:
'python generate_tfrecord.py --csv_input=labels.csv --output_path=test.record'

Momentan muss der Pfad zu den Bildern noch manuell im Skript geändert werden(Zeile 90).

Netzwerk trainieren:

Die zuvor erzeugten tfrecord Dateien, müssen in das Verzeichnis softwareprojekt2018/NeuralNetwork/Detection/object-detection/data gelegt werden.
Die Dateien müssen train.record und test.record heißen.

Nun muss das[vortrainierte Netzwerk](http: // download.tensorflow.org/models/object_detection/ssdlite_mobilenet_v2_coco_2018_05_09.tar.gz) heruntergeladen werden.

Dieses muss bis auf die pipeline.config nach softwareprojekt2018/NeuralNetwork/Detection/object-detection/SSDLite kopiert werden.

Nun startet das Training: 'python train.py  --logtostderr --train_dir=training/ --pipeline_config_path=SSDLite/pipeline.config'

Bei bedarf kann zusätzlich eine Evaluirung ausgeführt werden mit:
'python eval.py     --logtostderr     --pipeline_config_path=SSDLite/pipeline.config     --checkpoint_dir=training/     --eval_dir=training/'
Dies rechnet jedoch auch auf der Grafikkarte/CPU und sollte nicht konstant ausgeführt werden!

Mit 'tensorboard --logdir='training/'' kann nun eine Tensorboard Session gestartet werden, welche nützliche Graphen im Webbrowser visualisiert.

Training beendet:
Wenn das Training beendet ist, muss das Netzwerk noch exportiert werden mit:
'python object_detection/export_inference_graph.py - -input_type image_tensor - -pipeline_config_path object_detection/SSDLite/pipeline.config - -trained_checkpoint_prefix object_detection/training/model.ckpt-49297 - -output_directory object_detection/output/
'
