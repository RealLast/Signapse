# Daten lablen
Für das Labeln der aufgenommenen Daten sind folgende Schritte zu unternehmen:

## Verkehrszeichen im Bild
Um aus den gesammelten Daten die Bilder zu extrahieren, welche Verkehrszeichen enthalten, ist folgendes Vorgehen angedacht:

1. Video in Einzelbilder umwandeln.

    ```bash
    python video_to_frames.py -i video_name.mp4 -o Ordner-Name
    ```
    
2. In dem Ordner (in welchem sich die Einzelbilder befinden) befindet sich eine Datei names 'delete.csv'. Dort werden die Bereiche angegeben, in denen keine Verkehrszeichen sichtbar sind.
3. Alle Bilder in beliebigem Bildprogramm öffnen.
     
    3.1. Schaut euch das Bild an und entscheidet, ob ein Verkehrszeichen erkennbar ist oder nicht.

    3.2. Wenn kein Verkehrszeichen erkennbar ist, merkt euch die erste Position, wo kein Verkehrszeichen sichtbar ist.
    
    3.3. Geht nun die Bilder durch, bis ihr ein Verkehrszeichen findet und merkt euch die Nummer des Bildes, wo ihr dies gefunden habt.
    
4. Nun habt ihr einen Bereich, in welchem sich keine Verkehrszeichen befinden.
   Beispiel: Das erste Bild, in welchem **kein Verkehrszeichen** mehr gefunden wurde, war Bild `frame202.png` und **das erste, wo wieder eins sichtbar war**, `frame242.png`.
             Nun wird in der `delete.csv` in einer neuen Zeile `202,242` geschrieben.
             Die erste Zahl ist dabei inklusive (`frame202.png` wird später gelöscht) und die zweite exklusive(`frame242.png` wird nicht mehr gelöscht).  
             **!!! IHR NOTIERT _LÖSCH-BEREICHE_. DIE BILDER SIND NACH DEM AUSFÜHREN DES SKRIPTES DAUERHAFT _GELÖSCHT_ !!!**
5. Dies ist jetzt für alle in Schritt 1 extrahierten Bilder durchzuführen. Es ist dabei zu beachten, dass jeder Bereich in eine eigene Zeile geschrieben wird und sich sonst keine weiteren Zeichen in der Zeile befinden.
6. Wenn dies getan wurde und in der `delete.csv` alle zu löschenden Bilder drin stehen, kann man diese wie folgt löschen.

    ```bash
    python delte_from_csv.py -i Ordner-Name
    ```
7. Ist dies mit allen zugewiesenen Videos passiert, ist die Aufgabe abgeschlossen.    

## Verkehrszeichen klassifizieren

Der Datensatz, welcher mit der vorhergehenden Aufgabe auf Bilder mit Verkehrszeichen reduziert wurde, ist nun zu klassifizieren.

Dafür ist folgende Einrichtung notwendig:

### Installation von LableImg
**[Windows]**
1. Rechtsklick auf `setup_labelImg.ps1`
2. Mit PowerShell ausführen

**[Linux]**

**TODO**

### Klassifizieren
Nun ist die Software LableImg installiert. Öffnet diese mit:


1. Doppelklick auf den Ordner `LabelImg`
2. Doppelklick auf `labelImg.exe` um LableImg zu starten
3. Klickt oben in der Toolbar auf `View` und aktiviert `Auto Saving`. Dies ermöglich euch nur einmal den Speicherort festzulegen.
4. Klickt auf `Open Dir` und wählt den Ordner aus, welcher die Rohdaten enthält.
5. Nun sind die Bilder geladen und man kann mit `Next Image` und `Prev Image` durch diese navigieren.
6. Nun könnt Ihr mit `Create RectBox` ein Rechteck um ein Verkehrszeichen ziehen. Zoomed am Besten mit `Strg+Plus` oder `Strg+Scroll Rad` an diese heran, um die Box korrekt zu setzen.

    **So ist es falsch:**
    
    ![wrong box](https://cdn.fischedick.de/SWP/Images/wrong.png)
    ![wrong box|small](https://cdn.fischedick.de/SWP/Images/wrong2.png)
    
    **So ist es richtig**
    
    ![correct box|50%](https://cdn.fischedick.de/SWP/Images/correct.png)
    
7. Weist in dem sich nun öffnenden Fenster dem gerade markierten Schild die korrekte Klasse zu und bestätigt eure Wahl mit `OK`.
    
    **Achtung**, die Klasse `(Sonstiges)` ist für Schilder gedacht, welche für die Geschwindigkeit keine Bedeutung haben, z.B. das Vorfahrtsschild. Die Klasse `(Nicht in dieser Liste, aber relevant)` ist für Zeichen, welche die Geschwindigkeit regulieren (oder entsprechende Zusatzschilder). Diese Klassen müssen ergänzt werden, meldet euch also bei Robert und Söhnke, wenn ihr fertig seid, damit sie die Klasse ergänzen können und entsprechende Bilder nachlabeln.
8. Geht nun zum nächsten Bild. Beim ersten Bild werdet ihr nach einem Speicherort gefragt. Die Label speichert ihr am Besten in einem Ordner `Lable` in dem Ordner, in welchem sich die Bilder befinden.
9. Wiederholt Schritt 5-8.

# FAQ

**Ab wann gilt denn ein Schild als sichtbar?**  
_Sobald du es mit bloßem Auge gerade so lesen/eindeutig indentifizieren kannst(Zoomen ist erlaubt!)._
  
**Was ist mit Schildern die nur teilweise zu sehen sind?**  
_Wenn nur ein kleiner Teil fehlt (so um die 5-10%) kann das Schild mit gelabelt werden._  
_Sollte nur die Hälfte oder gar noch weniger zu sehen sein, wird das Schild ignoriert._

**Ich habe ein Schild das verdreht ist. Was mach ich damit?**  
_Ähnlich wie bei den Teilaufnahmen: Wenn die Rotation nur gering ist und die Frontfläche noch gut sichtbar ist, wird es gelabelt._  
_Ist das Schild hingegen stark verdreht ignorieren wir es. Insbesondere gelten Schilder, welche um 90° zur Fahrbahn verdreht sind nicht für den fließenden Verkehr und wir wollen das Netzwerk ja nicht darauf trainieren unwirksame Schilder zu erkennen._
  
**Ich habe Schilder gefunden die die Geschwindigkeit nicht betreffen. Darf ich die ignorieren?**  
_Nein. Wir brauchen **ALLE** Schilder für den Datensatz, damit das Netzwerk lernt welche Schilder wichtigen Schildern ähnlich sehen, aber ignoriert werden müssen._
_Was wir nicht labeln sind die nicht beschrifteten Rückseiten von Schildern._ 

**Auf meinem Bild sind Leuchttafeln zu sehen. Werden die gelabelt?**
_Ja. Wir markieren sie auf jeden Fall mit. Ob wir genügend davon in den Datensatz bekommen zeigt sich noch, aber sie kommen erstmal mit rein._


_Markiert **ALLE SCHILDER** bitte als ihre zugehörige Klasse oder, wenn diese nicht vorhanden ist, als "(Sonstiges <Farbe/Art>)"._
