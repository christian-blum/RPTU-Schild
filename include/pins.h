#ifndef __PINS_H
#define __PINS_H

// Welcher GPIO-Pin ist an die NeoPixels angeschlossen?
#define LED_PIN 23   // Wenn man MOSI benutzt, kann SPI in Hardware laufen
#define SEED_PIN A0  // von welchem Analog-Eingang nehmen wir Rauschen?

#define EINAUS_PIN 32
#define HELLER_PIN 26
#define DUNKLER_PIN 33
#define EFFEKTE_EINAUS_PIN 25
#define BT_EINAUSPAIRINGCLEAR_PIN 27 // 1x: Status, 2x: ein/aus, 3x: alle gepairten Geräte löschen

#endif