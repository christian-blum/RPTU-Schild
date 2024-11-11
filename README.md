# RPTU-Schild

Es war einmal vor langer Zeit in einer weit entfernten Galaxis, da gab es eine
Technische Hochschule Kaiserslautern. Alle waren glücklich und zufrieden
(also... bis auf den baulichen Zustand und den ÖPNV). Doch
weil das den Menschen nicht gut tut, entsannen die Weisen, dass es eine gute
Idee sei, sie mit dem Campus Landau der Universität Koblenz-Landau
zusammenzulegen. Es entstand die Rheinland-Pfälzische Technische Universität
Kaiserslautern-Landau, kurz RPTU. Und damit auch jeder merke, was für eine
willkürliche Idee das ist, entsann man ein Logo, das diese Willkür perfekt
widerspiegelt, indem es jede:r (fast) so malen darf wie sie/er/und alle
dazwischen und außerhalb gerne möchte.

## Bauanleitung (in Kurzform)

### Teileliste

Ihr benötigt:

```
  3x LED Panel WS2812B 24x8, https://www.amazon.de/dp/B088K1KDW5
  1x NodeMCU ESP32 z.B. https://www.amazon.de/AZDelivery-NodeMCU-Development-Nachfolgermodell-ESP8266/dp/B074RG86SR
  1x Netzteil 5V mindestens 100W, z.B. https://www.amazon.de/dp/B078B5BWDF
  1x Kaltgerätebuchse, z.B. https://www.amazon.de/dp/B09TXPLGD8
  4x Tasten, z.B. https://www.amazon.de/dp/B081TV622R
  Jumper Wires, z.B. https://www.amazon.de/dp/B07KCFG5YX
  1x Schottky-Diode, z.B. SB130 (ihr könnt auch eine 1N400x nehmen)

  und natürlich
  1x Kaltgerätekabel
  1x MicroUSB-Kabel
```

Ein Gehäuse ist optional. Euch fällt da schon was ein.

### Verschaltung

Ihr schaltet die LED-Panels hintereinander, indem ihr Ausgang und GND des ersten
mit dem Eingang und GND des zweiten vebindet, und natürlich Ausgang und GND des zweiten
mit dem Eingang und GND des dritten. Die 5V Stromversorgung schaltet ihr parallel
und verbindet sie mit dem Netzteil. Schnappt euch eine Sperrholzplatte, bohrt
Löcher für die Kabel rein, und ordnet darauf die LED-Panels übereinander an, so dass
eine 32x24 Matrix entsteht. Den Eingang des ersten LED-Panels verbindet ihr über
Jumper Wires mit dem NodeMCU-Modul. An den 5V-Anschluss des Panels lötet ihr die
Anode der Diode, der Jumper Wire kommt an die Kathode (das Ende mit dem Ring).
Verbindet die Kathode mit dem `5V` Anschluss des NodeMCU-Moduls. GND verbindet ihr
natürlich mit `GND` des NodeMCU-Moduls, und der Eingang des LED-Panels kommt an
`GPIO23` (MOSI oder COPI). Die vier Tasten verschaltet ihr so, dass sie mit einem
ihrer Anschlüsse verbunden sind (der kommt an den `3V3` Anschluss des NodeMCU). Die
anderen Anschlüsse kommen an: `GPIO32` (Ein/Aus), `GPIO33` (dunkler), `GPIO26` (heller)
und `GPIO25` (Effekte ein/aus). Das Netzteil verbindet ihr mit der Kaltgerätebuchse
(und ja, bitte mit Schutzleiter!), und das Kaltgerätekabel und das USB-Kabel ... okay,
das bekommt ihr hin. Fertig, mehr braucht ihr nicht!

### Ein paar Details

Die LEDs werden über ein einfaches serielles Protokoll angesteuert, das mit
freundlicher Hardware-Unterstützung des SPI-Ausgangs erzeugt wird. Dafür wird die
Bibliothek FastLED verwendet. Die Panels sind ziemlich wild verschaltet, nämlich
in Schlangenlinie, weshalb man im Code ein bisschen Akrobatik bei der Ansteuerung
treiben muss.

Die billigen Tasten prellen natürlich wie Sau, und vielleicht wundert ihr euch,
dass keine Entprellungsschaltung vorgesehen ist. Wir machen das in Software, das
funktioniert tatsächlich besser als in Hardware und kostet nichts, vor allem keine
Arbeit. Das macht die Bibliothek KeyDebouncer für uns.

## Inbetriebnahme

### Compilieren

Ihr braucht VS Code mit Platform IO. Fangt erst gar nicht mit der Arduino IDE an,
ihr raubt euch damit nur den letzten Nerv. Clonet das Git-Repository, dann clickt
ggf. auf das Ameisengesicht links und öffnet den Folder. Mit Strg-Alt-U compiliert
ihr und übertragt die erzeugte firmware.bin ins Flash des NodeMCU-Moduls. Das ist
wirklich schon alles. Platform IO macht alles andere für euch im Hintergrund.

### Einschalten

Ok, schafft ihr.

### Bedienen

Mit den Tasten könnt ihr die LEDs ein-/ausschalten, die Helligkeit verändern und
die Effekte autivieren/deaktivieren. Der Ein/Aus-Knopf hat noch eine wichtige
Sonderfunktion: wenn ihr die Schaltung komplett vom Strom trennt, den Ein/Aus-Knopf
festhaltet, den Strom einschaltet, und den Knopf dann erst nach ein paar Sekunden
wieder loslasst, wird alles auf Werkseinstellungen zurückgesetzt. Ansonsten
braucht ihr die Knöpfe eigentlich nicht wirklich, denn:

### Konfigurieren und Fernbedienen

Scannt mal die WLANs mit eurem Smartphone. Ihr werdet eins finden, das einen seltsamen
Namen `ESP32-xxxxxx` und kein Passwort hat. Verbindet euch damit (und erlaubt, dass
das Netzwerk eben ohne Internetverbindung ist). Dann öffnet den Browser und gebt
die IP `192.168.4.1` ein.

Als Allerallererstes legt ihr bitte unter `Setup`, `Administratoreinstellungen` einen
Benutzernamen und ein Passwort fest. Damit schützt ihr euer Schild vor Spielkindern,
die euch ärgern wollen.

Dann geht ihr in die `Netzwerkeinstellungen`, wählt euer WLAN aus, gebt euer
WLAN-Passwort ein (leider kann das NodeMCU nur WPA2-PSK) und clickt dann auf
`Speichern und Neustart`. Euer Schild verbindet sich jetzt mit eurem WLAN und kriegt
eine IP über DHCP. Ihr könnt es (wenn euer Smartphone wieder mit eurem WLAN verbunden
ist) ab sofort unter `ESP32-xxxxxx.local` (oder was euer Netz als Domain verwendet)
ansprechen. Gegebenenfalls müsst ihr in eurem Router nachsehen, welche IP es
gekriegt hat. Oder ihr verbindet das NodeMCU-Modul per USB mit dem PC und lest
über den Serialport bei 115200 bps mit. (Denkt daran: wenn ihr euch ins Knie
schießt und euch total verheddert: der Ein/Aus-Knopf ist euer Retter).

Wenn aus irgendeinem Grund die Verbindung mit eurem WLAN scheitert, baut das Schild
wieder sein eigenes WLAN aus.

Ihr habt übrigens richtig gesehen: Da stand `OTA-Update`. Statt über USB-Kabel könnt
ihr die Datei `firmware.bin`, die im Verzeichnis `.pio/build/esp32dev` entsteht,
auch einfach hochladen, wenn es eine neue Version gibt.

Wie man das Schild nun fernsteuert und konfiguriert, das findet ihr sicher selbst
heraus. Ihr könnt so ziemlich alles einstellen. An verschiedenen Stellen gibt es
`Defaults` Buttons - damit könnt ihr die Defaults für einen Übergang oder einen
Effekt wieder einstellen. Eingaben und Änderungen werden nur übernommen, wenn ihr
auf `Ändern` clickt, und es kann mitunter ein paar Sekunden dauern, bis ihr etwas
davon bemerkt, weil laufende Effekte/Übergänge nicht beeinflusst werden.

Noch eine kleine Information zu benutzerdefinierten Laufschriften. Durch eine Macke
in der Preferences-Bibliothek des ESP32 ist irgendwann Schluss mit neuen Laufschriften,
weil die Bibliothek die alten nicht ganz vergessen und sich nicht beliebig
viele merken kann. Dann hilft nur noch ein Werksreset. Aber wir reden von 200+.

## Copyright

Nett, dass ihr fragt.

```
RPTU-Schild (Hardware und Software)
Copyright (C) 2024 Tilman Pfersdorff und Christian Blum

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <https://www.gnu.org/licenses/>.
```

Kurzfassung:

Ihr könnt es nachbauen, ihr könnt mit der Software spielen, ihr könnt es
weitergeben, auch in der modifizierten Form, kostenlos oder auch nicht,
aber es war unsere Idee und wir haben es zuerst gebaut, und ihr behauptet
nichts anderes und gebt anderen die Möglichkeit, mit unserer oder eurer
Fassung dasselbe zu tun wie ihr. Und wenn euch was um die Ohren fliegt
ist das euer Problem, ihr seid schließlich erwachsen.

## Über die Autoren

Tilman Pfersdorff studiert Elektrotechnik am KIT in Karlsruhe. Er hatte
die Idee, für seinen Bruder, der an der RPTU studiert, ein solches
Logo-Veräppelungs-Schild zu basteln, und er hat das Projekt anfangs
vorangetrieben und den Prototyp gebaut. Wie das halt so ist hat die Zeit
nicht gereicht, man hat als Student ja auch noch Anderes zu tun, zum
Beispiel studieren.

Ihr Papa Christian Blum hat ebenfalls Elektrotechnik studiert und bastelt
seit 46 Jahren (ja, wirklich) mit Mikrocontrollern. Zwischendurch war er
mal durch langweilige Dinge wie Job und spannende Dinge wie Kinderaufzucht
zu sehr abgelenkt, aber es hat viel Spaß gemacht, wieder einzusteigen.
Wirklich toll, was es inzwischen für coole Spielzeuge gibt. Er hat die
Software in der jetzigen Form geschrieben.

Ihr erreicht uns unter der E-Mail-Adresse `bastelkeller@pf-b.de`.