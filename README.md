# Softwareprojekt 2018
[![build status](https://nikrgl.informatik.tu-ilmenau.de/students/softwareprojekt2018/badges/master/build.svg)](https://nikrgl.informatik.tu-ilmenau.de/students/softwareprojekt2018/commits/master)

Strassenschilderkennung mittels Smartphone

# Hinweise aus e-Mail bitte beachten!
# Keine Binärdateien einchecken!

## Struktur

* `/Documents` - Softwareprojekt Dokumente
* `/Documents/Entwicklerdokumentation` - Entwicklerdokumentation für _Java_, _C++_ und _Python_
* `/App/Android/TSD` - Signapse Android Studio Projekt
* `/RoadSignAPI_Standalone_Tensorflow` - RoadSignAPI und FilterManagementLibrary (C++) + Ubuntu CMake Umgebung 
* `/RoadSignAPI_Standalone_Tensorflow/build_for_android` - Zusätzliche NDK Build Umgebung für die Android Library (C++ Umgebung für VerkehrszeichenAPI und FilterVerwaltungsBibliothek)
* `/RoadSignAPI_Standalone` - Überbleibsel der ursprünglichen C++ Umgebung (CMake), welche auf TensorFlow Lite basiert. Beinhaltet eine frühe Version der FilterManagementLibrary, welche Klassen für die Verwendung von TensorflowLite bereitstellt (lauffähig!)
* `/NeuralNetwork/Detection` - Trainingsumbebung für den Detektor
* `/NeuralNetwork/Classification` - Trainingsumbebung für den Klassifikator
* `/ToolChain` - Weitere Skripte, welche z.B. für die Datensatzaufbereitung genutzt werden können

## Installation

### Signapse Android App

#### Installation aktueller Version über Play Store

1. Über das Smartphone die [Play Store Seite](https://play.google.com/store/apps/details?id=de.swp.tsd.trafficsigndetection) aufrufen
2. Die App installieren

#### Installation Version 7/8 per APK Sideloading

1. Je nach Smartphone APK Datei den Git-Externen Dateien entnehmen und in das Download-Verzeichnis des Smartphones kopieren:
   * `signapse-armeabi-v7a-release-v7.apk` (ARMv7)
   * `signapse-arm64-v8a-release-v8.apk` (ARMv8)
2. In den Einstellungen des Smartphones
   * Android 7.x: Unter `Sicherheit` `Installation aus Unbekannten Quellen` erlauben
   * Android 8.x+: Unter `Unbekannte Apps installieren` `Unbekannte Apps installieren` für Dateimanager der Wahl erlauben
3. Über den Dateimanager auf dem Smartphone die APK lokalisieren und installieren

#### Kompilieren über Android Studio

1. Git Repository clonen:
```bash
git clone https://nikrgl.informatik.tu-ilmenau.de/students/softwareprojekt2018.git
```

2. `softwareprojekt2018_external_files.zip` den Git-Externen Dateien entnehmen und so in das kopierte Repositoryentpacken, damit die Ordnerstruktur übereinstimmt
3. Android Studio installieren
4. NDK [Version 15c](https://developer.android.com/ndk/downloads/older_releases) in einen Ordner der Wahl entpacken
5. Von Android Studio aus den Ordner `/App/Android/TSD` als Projekt öffnen
6. (Für korrekte Git Einbindung beim ersten Start unter `Event Log`, wenn dort ein Git-Hinweis erscheint, das root hinzufügen)
7. Unter `File > Project Structure > SDK Location > Android NDK location` den NDK 15c Ordner auswählen
8. In den Entwicklereinstellungen des Smartphones ADB aktivieren, das Smartphone mit dem PC verbinden und ADB für den PC erlauben
9. Den `Run` Button klicken, das angeschlossene Gerät auswählen und `Ok` klicken
10. Die App sollte nun kompiliert und auf dem Gerät installiert werden

## Neuronale Netzwerke

### Detektion

Wie das verwendete Netzwerk trainiert werden kann, ist in der [ReadMe](https://nikrgl.informatik.tu-ilmenau.de/students/softwareprojekt2018/tree/feature/NeuralNetwork/Detection#neural-network-aus-der-2-phase) im Unterverzeichnis [`NeuralNetwork/Detection/`](https://nikrgl.informatik.tu-ilmenau.de/students/softwareprojekt2018/tree/feature/NeuralNetwork/Detection) erklärt.
Zusätzlich wird dort erläutert, wie das Netzwerk als Protobuf Datei für die Verwendung in der App exportiert werden kann.

### Klassifikation

Zum Trainieren des Klassifikator muss das Skript [`NeuralNetwork/Classification/mobilenetv2.py`](https://nikrgl.informatik.tu-ilmenau.de/students/softwareprojekt2018/blob/feature/NeuralNetwork/Classification/mobilenetv2.py) per Kommandozeile aufgerufen werden.

Im Kopf des Skriptes befinden sich Parameter, welche für das Training bei bedarf angepasst werde können.

Die wichtigsten sind hierbei:
```bash
train_data_path = ’/datasets/swp2018/split_img/train’ 
validation_data_path = ’/datasets/swp2018/split_img/validation’
```
Welche die Pfad zu den Trainings bzw. Validierungsdaten angeben.

Außerdem können noch die Anzahl der Datein, Bildgröße und die Anzahl der Klassen bestimmt werden.

```bash
file_count_train = 134443 
image_size = 96 
classes = 47
```
Diese werden benutzt um die Steps zu bestimmen, welche für einen Batch benutzt werden. 
Eine automatische Bestimmung kann leicht automatisiert werden, wurd jedoch aus Zeitgründen weggelassen.

Außerdem lassen sich übliche Trainingsparameter einstellen:
```bash
epochs = 200 
batch_size = 128 
learn_rate = 0.045 
decay = 0.00004 
momentum = 0.9
```

Um ein trainiertes Netzwerk in der App verwenden zu können muss dies in eine Protobuf Datei umgewandelt werden.
Dies geht mit dem Skript [`NeuralNetwork/Classification/keras_to_tensorflow.py`](https://nikrgl.informatik.tu-ilmenau.de/students/softwareprojekt2018/blob/feature/NeuralNetwork/Classification/keras_to_tensorflow.py) wie folgt:
```bash
python keras_to_tensorflow.py -input_model weights.hdf5
```
Hierbei bezeichnet weights.hdf5 die trainierten und gespeicherten Keras Gewichte.
Nach der Ausführung befindet sich eine .pb Datei im selben Ordner, welche als Klasifikator in der App verwendet werden kann.

## C++ Umgebung für VerkehrszeichenAPI und FilterVerwaltungsBibliothek
Das Folgende bezieht sich alles auf die Umgebung innerhalb des Ordners /RoadSignAPI_Standalone_Tensorflow.
Für die Kompilierung der App ist dies alles irrelevant, da dies aus Android Studio heraus geschieht.
Will man aber z.B. auf Desktop Systemem kompilieren oder entwickeln (oder die VerkehrszeichenAPI dort verwenden), dann bitte folgendes beachten:

Für die Entwicklung unter Ubuntu bzw. x86_64 Linux wurde eine CMake basierte Entwicklungsumgebung aufgesetzt. 
Die VerkehrszeichenAPI und FilterVerwaltungsBibliothek wurden unter Ubuntu (18.04, 17.10, 16.10 und 16.04) entwickelt und getestet. 
Die hierfür benötigten statischen Bibliotheken (TensorFlow, ProtoBuf, Nsync) liegen im Ordner dependencies/lib, die
dazugehörigen header Dateien in dependencies/lib (nachdem die Git-externen Dateien `softwareprojekt2018_external_files.zip` entpackt wurden, siehe oben).

Vor dem ersten Kompilieren sollte der Ordner "CMakeFiles" und die Datei "CMakeCache.txt" gelöscht werden (wenn vorhanden)! 
Sonst wird das Kompilieren mit CMake mit an Sicherheit grenzender Wahrscheinlichkeit fehlschlagen!

Anschließend navigiert man mit einem Terminal seiner Wahl in den Ordner und führt einmalig
```bash
cmake .
```
aus

Dies erstellt die make Datei die von CMake für die Kompilierung verwendet wird.
Für das kompilieren dann einfach 

Anschließend navigiert man mit einem Terminal seiner Wahl in den Ordner und führt einmalig
```bash
make all
```
verwenden. Man kann das ganze auch sehr einfach in Eclipse integrieren, indem man dort "import from existing cmake project" auswählt.


Nun noch ein paar allgemeine Informationen:

Die TensorFlow Header müssen in einer ganz bestimmten Art und Weise eingebunden werden, dasselbe gilt für die Librarys
wobei bei diesen die Reihenfolge der Einträge eine essentielle Rolle spielt! Man sollte also in der CMakeLists.txt 
z.B. nicht schreiben, dass ProtoBuf vor TensorFlow gelinkt werden soll, weil dies zu (Linker) Fehlern beim Kompilieren führt.
Am Besten man lässt hier alles wie es ist, was die externen Abhängigkeiten betrifft.

Die Quelldateien der von uns entwickelten API und Bibliothek liegen unter `/header_files` und `/source_files`.
Alle Änderungen, die hier passieren, werden automatisch von der Android Studio Umgebung übernommen!
Diese verweist direkt auf die hier liegenden Dateien, weshalb die Ordnerstruktur des Projektes unbedingt beibehalten
werden sollte!
Falls man Teile davon Auslagern und in anderen Projekten verwenden möchte, dann kann man einfach die entsprechenden
Unterordner "FilterManagementLibrary" und "RoadSignAPI" kopieren.

Dieser Ordner beinhaltet außerdem ein Beispiel für die Verwendung der RoadSignAPI unter Ubuntu bzw. x86_64 Linux.
Hierfür wurde eine `main.cpp` nur für diesen Fall erstellt (haben wir z.B. benutzt um die Videos zu erstellen). 
In dieser wird die RoadSignAPI eingebunden und initialisiert. Es wird dann ein Video geladen (hardgecodet, kann man aber einfach ändern, einfach die Kommentare beachten)
und abgespielt, wobei alle KLASSIFIZIERTEN Verkehrszeichen mit Bezeichnung und Confidence (des Klassifikators!) angezeigt werden.
Alle Verkehrszeichen, die am selben Mast hängen werden dabei außerdem speziell umrahmt, und die vermutete Position des Pfahls / Mastes wird angezeigt (sehr grob).
Hierfür (und für die Verwendung der RoadSignAPI generell) muss aber (wenigstens) OpenCV 3 installiert sein (CMake sucht die entsprechenden Verzeichnisse selbst, daher reicht ein einfaches
entpacken oder selbst kompilieren der OpenCV Abhängigkeiten nicht aus!).
Ich verweise mal [hier](https://docs.opencv.org/3.0.0/d7/d9f/tutorial_linux_install.html) drauf.


Noch ein Hinweis: Will man die FilterVerwaltungsBibliothek OHNE OpenCV Unterstützung verwenden, dann bitte die Datei `/source_files/FilterManagementLibrary/TensorflowOpenCVUtils.cpp` 
nicht einbinden :)

### build_for_android
Dieser Ordner ist eigentich auch mehr ein Überbleibsel. Er kann allerdings genutzt werden, um die Android NDK Library, welche später von 
der Android App geladen wird (in Java, um die C++ Funktionen einzubetten), unabhängig von Android Studio zu kompilieren. Hierfür wurde eine clang Umgebung (`Android.mk`) aufgesetzt.
Zum Kompilieren bitte zuerst das Script `set_ndk_path_and_make` editieren, hier muss der NDK Pfad eingetragen werden!
Danach kann man es einfach ausführen. Erstellt wird dabei eine shared library für armeabi-v7a.
arm64-v8a wäre möglich, wenn man dies in der `Android.mk` definiert. Ist in unseren Tests aber fehlgeschlagen.

Trivia: Wir haben diese Umgebung ursprünglich erstellt, weil das Kompilieren der TensorFlow Libraries für arm64-v8a ständig Fehler warf.
Prinzipiell ist die armeabi-v7a Android Library auch unter arm64-v8a lauffähig, aber wenn man diese direkt aus Android Studio heraus kompiliert, 
wird ein enstprechender Fehler ausgeworfen, wenn man keine passenden Libraries für die entsprechende Architektur angibt.
Wir haben das ganze umgangen, indem wir die Anweisung, die Library extern zu kompilieren, entfernt, diese dann 
mit dieser clang Umgebung erstellt und anschließend in das Android Studio Projekt kopiert und

Nach dem Kompilieren mit dem Skript liegt das Ergebnis unter `build_for_android/libs/armeabi-v7a`. Die Datei heißt `libnative_lib.so`.

## Usage

### Signapse Android App
* Siehe [Benutzerhandbuch](https://nikrgl.informatik.tu-ilmenau.de/students/softwareprojekt2018/tree/master/Documents/Benutzerhandbuch)


## Changelog

* Siehe [Tags](https://nikrgl.informatik.tu-ilmenau.de/students/softwareprojekt2018/tags)
