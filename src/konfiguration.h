#ifndef __KONFIGURATION_H
#define __KONFIGURATION_H

#include <Arduino.h>
#include "led_matrix.h"


typedef struct sPosition Position;

struct sKonfiguration {
  struct sCRGBA schriftfarbe;
  struct sCRGBA hintergrundfarbe;
  Position r;
  Position p;
  Position t;
  Position u;
  bool wrapX;
  bool wrapY;
  struct sKonfiguration *naechste; // wird für Slotmachine gebraucht
};

void konfiguration_rendern(struct sCRGBA *bitmap, struct sKonfiguration *k);
struct sKonfiguration *clone_konfiguration(struct sKonfiguration *k);
void uebergang_queueKonfiguration(struct sKonfiguration *k, int32_t milliseconds);
struct sKonfiguration konfiguration_wuerfeln();
bool farbenZuAehnlich(struct sCRGBA f1, struct sCRGBA f2);
uint8_t helligkeitEinerFarbe(struct sCRGBA farbe);
struct sCRGBA farbeAbdunkeln(struct sCRGBA farbe, uint8_t faktor);

#endif