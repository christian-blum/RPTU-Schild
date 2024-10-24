#include "konfiguration.h"
#include "einstellungen.h"
#include "defaults.h"
#include "rptu_stuff.h"
#include "cb_scheduler.h"


struct sKonfiguration konfiguration_wuerfeln() {
  struct sKonfiguration x;
  x.r.x = 0;
  x.p.x = 8;
  x.t.x = 16;
  x.u.x = 24;
  do {
    x.r.y = 8 * random(3);
    x.p.y = 8 * random(3);
    x.t.y = 8 * random(3);
    x.u.y = 8; // this is fixed
  }
  while ((x.r.y == x.p.y && x.p.y == x.t.y) || (x.p.y == 8 && x.t.y == 8));
  int sf;
  int hf;
  uint8_t erlaubt;
  do {
    do {
      sf = random(rptu_anzahl_farben);
      if (hintergrund_schwarz) hf = 10;
      else hf = random(rptu_anzahl_farben);
      erlaubt = rptu_farbkombination_erlaubt(hf, sf);
    }
    while (!erlaubt);
    x.schriftfarbe.crgb = FARBEN[sf];
    x.schriftfarbe.alpha = 0xff;  // eigentlich unnötig, wenn nur auf dem base layer
    x.hintergrundfarbe.crgb = FARBEN[hf];
    x.hintergrundfarbe.alpha = 0xff;  // eigentlich unnötig, wenn nur auf dem base layer
  } while (farbenZuAehnlich(x.schriftfarbe, x.hintergrundfarbe));
  if (HINTERGRUND_FAKTOR > 1 && helligkeitEinerFarbe(x.schriftfarbe) >= 64) x.hintergrundfarbe = farbeAbdunkeln(x.hintergrundfarbe, HINTERGRUND_FAKTOR);
  x.naechste = NULL; // wichtig... sonst kann die Slotmachine Mist bauen
  return x;
}

struct sKonfiguration *clone_konfiguration(struct sKonfiguration *k) {
  struct sKonfiguration *n;
  n = (struct sKonfiguration *) malloc(sizeof(struct sKonfiguration));
  memcpy(n, k, sizeof(struct sKonfiguration));
  return n;
}


void uebergang_queueKonfiguration(struct sKonfiguration *k, int32_t milliseconds) {
  struct sBitmap *b = (struct sBitmap *) malloc(sizeof(struct sBitmap));
  memset(b, 0, sizeof(struct sBitmap));
  struct sCRGBA *bitmap = (struct sCRGBA *) calloc(LED_COUNT, sizeof(struct sCRGBA));
  b->bitmap = bitmap;
  konfiguration_rendern(bitmap, k);
  b->milliseconds = milliseconds;
  base_queue(b);
}


bool farbenZuAehnlich(struct sCRGBA f1, struct sCRGBA f2) {
  int16_t diff = abs((int16_t)f1.r-(int16_t)f2.r) + abs((int16_t)f1.g-(int16_t)f2.g) + abs((int16_t)f1.b-(int16_t)f2.b);
  if (diff < 256) return true;
  return false;
}

uint8_t helligkeitEinerFarbe(struct sCRGBA farbe) {
  return (uint8_t) sqrt(0.299f*farbe.r*farbe.r + 0.587f*farbe.g*farbe.g + 0.114*farbe.b*farbe.b);
}

struct sCRGBA farbeAbdunkeln(struct sCRGBA farbe, uint8_t faktor) {
  CRGBA c;
  c.r = farbe.r / faktor;
  c.g = farbe.g / faktor;
  c.b = farbe.b / faktor;
  c.alpha = farbe.alpha;
  return c;
}



void serial_print_hex(uint8_t x) {
  if (x < 16) Serial.print("0");
  Serial.print(x, HEX);
}

void serial_print_crgba(struct sCRGBA c) {
  serial_print_hex(c.alpha);
  serial_print_hex(c.r);
  serial_print_hex(c.g);
  serial_print_hex(c.b);
}

/*
void konfiguration_base_dump() {
  Serial.println("base dump folgt");
  for (int y=0; y<LED_COUNT_Y; y++) {
    for (int x=0; x<LED_COUNT_X; x++) {
      CRGB c = base[y * LED_COUNT_X + x];
      serial_print_crgba(c);
      Serial.print(" ");
    }
    Serial.println();
  }
}
*/


void konfiguration_dump(struct sKonfiguration *k) {
  Serial.print("R x="); Serial.print(k->r.x); Serial.print(" y="); Serial.print(k->r.y);
  Serial.print("  P x="); Serial.print(k->p.x); Serial.print(" y="); Serial.print(k->p.y);
  Serial.print("  T x="); Serial.print(k->t.x); Serial.print(" y="); Serial.print(k->t.y);
  Serial.print("  U x="); Serial.print(k->u.x); Serial.print(" y="); Serial.print(k->u.y);
  Serial.println();
  Serial.print("Schriftfarbe: "); serial_print_crgba(k->schriftfarbe); Serial.print("  Hintergrundfarbe: "); serial_print_crgba(k->hintergrundfarbe); Serial.println();
}

void konfiguration_bitmapFuellen(struct sCRGBA *bitmap, struct sCRGBA farbe) {
  for (int i = 0; i < LED_COUNT; i++) {
    *(bitmap++) = farbe;
  }
}

void konfiguration_zeichenRendern(struct sCRGBA *bitmap, const uint8_t *zeichenDefinition, struct sPosition wo, struct sCRGBA farbe, bool wrapX, bool wrapY) {
  for (int16_t y = wo.y; y < wo.y + 8; y++) {
    uint8_t b = *(zeichenDefinition++);
    int16_t ypos = y;
    if (wrapY) ypos = y % LED_COUNT_Y;
    if (ypos >= 0 && ypos < LED_COUNT_Y) { 
      uint8_t m = 0x80;
      for (int16_t x = wo.x; x < wo.x + 8; x++) {
        if (b & m) {
          int16_t xpos = x;
          if (wrapX) xpos = x % LED_COUNT_X;
          if (xpos >= 0 && xpos < LED_COUNT_X && ypos >= 0 && ypos < LED_COUNT_Y) bitmap[ypos * LED_COUNT_X + xpos] = farbe;
        }
        m >>= 1;
      }
    }
  }
}

 void konfiguration_rendern(struct sCRGBA *bitmap, struct sKonfiguration *k) {
  konfiguration_bitmapFuellen(bitmap, k->hintergrundfarbe);
  konfiguration_zeichenRendern(bitmap, CHARSET8_R, k->r, k->schriftfarbe, k->wrapX, k->wrapY);
  konfiguration_zeichenRendern(bitmap, CHARSET8_P, k->p, k->schriftfarbe, k->wrapX, k->wrapY);
  konfiguration_zeichenRendern(bitmap, CHARSET8_T, k->t, k->schriftfarbe, k->wrapX, k->wrapY);
  konfiguration_zeichenRendern(bitmap, CHARSET8_U, k->u, k->schriftfarbe, k->wrapX, k->wrapY);
}
