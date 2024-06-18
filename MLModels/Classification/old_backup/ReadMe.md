Basierend auf https://github.com/tensorflow/models/tree/master/research/slim

# Benutzung:

Die Bilder werden zunächst mit convert_to_tfrecord.py in das tfrecord format gebracht. 
Dazu muss vorher in convert_to_tfrecord.py und in trafficsigns.py die Parameter für
Größe der Trainings- bzw. Validierungsdaten, sowie die Anzahl der Klassen und der Shards angepasst werden.

Die Gewichte der Klassen müssen in train_image_classifier.py angepasst werden.
Danach erstelle man einen Unterordner data mit folgender Strucktur:

data:

    \pretrained-models          - Enthält die vortrainierten Netzwerke in jeweils einem Unterordner
        \mobilenet_v2_0.5_96    
        \mobilenet_v2_0.35_96
        \mobilenet_v2_0.75_96
        \mobilenet_v2_1.0_96
    \trafficsigns               - Enthält die Trainingsdaten im tfrecord Format
    \trained-model              - Wird während und nach dem Training das neu trainierte Netzwerk enthalten
    

Danach kann man mit hilfe der train.sh und der evaluate.sh das Netzwerk trainieren.