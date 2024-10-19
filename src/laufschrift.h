#ifndef __FONT5X7_H
#define __FONT5X7_H

#include <Arduino.h>
#include "led_matrix.h"

#define LAUFSCHRIFT_TEXTFRAGMENT_GROESSE (LED_COUNT_X / 6 + 3)

struct sLaufschrift {
  int16_t y;
  const char *text; // nullterminiert, mit genügend vielen Leerzeichen davor und dahinter (6)
  CRGBA schriftfarbe;
  CRGBA hintergrundfarbe;
  uint32_t millis; // pro Schritt nach links
  // werden berechnet
  uint16_t zeichenzahl;
  uint16_t count; // zählt bis 6x Zeichenzahl
  uint16_t count_ende;
  volatile bool semaphore;
  char textfragment[LAUFSCHRIFT_TEXTFRAGMENT_GROESSE];
};

void text_rendern(struct sCRGBA *bitmap, struct sPosition wo, struct sCRGBA schriftfarbe, struct sCRGBA hintergrundfarbe, char *text);
bool laufschrift_rendern(struct sLaufschrift *ls);

#endif