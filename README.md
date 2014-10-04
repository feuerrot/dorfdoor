# Türsteuerung
Can haz digitale Türsteuerung mit verschiedenen Inputs und Statusleds und
foo? Ja!

Die [Tür des Chaosdorf](https://wiki.chaosdorf.de/Chaosdoor) hat einen 
eingebauten Motor von Fuhr - den Multitronic 881 und da alle Funkanwendungen
(insbesondere der Funk-Fingerabdrucksensor) auf unbekannter Technik basieren
und damit erstmal als für einen Hackerspace ungeeignet gelten, muss man das
natürlich selber bauen.

v1 umfasste eine Lochrasterplatine mit ATMega8, der über Optokoppler den
Potentialfreien Eingang des Türmotors ansteuerte. Ein Problem war unter
anderem, dass der komplette Öffnungs- und Schließvorgang incl. Verriegeln
dazu führte, dass nach gerade mal 3 Jahren ein Zahnrad brach.

Die aktuelle v2 trennt eigentliche Steuerung und Boardinterface auf zwei
Platinen auf. Der Mikrocontroller ist zusammen mit der verbundenen Seagate
Dockstar und passendem Netzteil für alles und Tür in einem Gehäuse. Die
Ansteuerung der Tür und das damit verbundene Problem bleibt bestehen.

@penma hat aber dankenswerterweise das Protokoll der Tür im für unseren
Zweck ausreichenden Maße reverse engineered, um die gewünschte
Tagesfallenfunktion auch ohne Zusatzplatine für wahrscheinlich mehrere
hunderte Euro zu erlangen. Die dafür notwendige Hardwareänderung wird in
Version 2 reingepatcht und in Version 3 dann standardmäßig vorhanden sein.

<!-- TODO: Noch die Bilder vom Layout reinpacken
## Schaltplan

### Sensor
![Schaltplan des Sensors am Gaszähler](img/schaltplan_sensor.png)

### Konverter
![Schaltplan des Konverters](img/schaltplan_s0-converter.png)

### Beides
![Komplettschaltplan](img/schaltplan.png)

## Layout

### Sensor
![Layout des Sensors am Gaszähler](img/layout_sensor.png)

### Konverter
![Layout des Konverters](img/layout_s0-converter.png)

### Beides
![Komplettlayout](img/layout.png)
-->
