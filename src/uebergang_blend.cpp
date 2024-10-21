#include <Arduino.h>

#include "uebergang.h"
#include "uebergaenge.h"
#include "uebergang_blend.h"
#include "led_matrix.h"
#include "konfiguration.h"
#include "einstellungen.h"


Uebergang_Blend::Uebergang_Blend(bool aktiv, uint16_t gewichtung, uint16_t steps, uint16_t delay) : Uebergang_sd(aktiv, gewichtung, steps, delay) {
  name = (char *)"Blend";
  beschreibung = (char *)"Weiche Überblende von einem zum anderen Zustand.";
  tag = (char *)"blend";
}

bool Uebergang_Blend::doit(struct sKonfiguration *alt, struct sKonfiguration *neu) {
  if (blend_step == 0) {
    aBitmap = (struct sCRGBA *) calloc(LED_COUNT, sizeof(struct sCRGBA));
    konfiguration_rendern(aBitmap, alt);
    nBitmap = (struct sCRGBA *) calloc(LED_COUNT, sizeof(struct sCRGBA));
    konfiguration_rendern(nBitmap, neu);
  }

  struct sBitmap *b = (struct sBitmap *) malloc(sizeof(struct sBitmap));
  memset(b, 0, sizeof(struct sBitmap));
  struct sCRGBA *bitmap = (struct sCRGBA *) calloc(LED_COUNT, sizeof(struct sCRGBA));
  b->bitmap = bitmap;
  for (int p = 0; p < LED_COUNT; p++) {
    bitmap[p] = morph_color(aBitmap[p], nBitmap[p], blend_step, steps);
  }
  blend_step++;
  b->milliseconds = (blend_step == steps + 1) ? konfiguration_pause : delay;
  base_queue(b);

  if (blend_step >= steps + 1) {
    free(aBitmap); aBitmap = nullptr;
    free(nBitmap); nBitmap = nullptr;
    blend_step = 0;
    return true;
  }
  return false;
}

Uebergang_Blend::~Uebergang_Blend() {
  if (aBitmap) free(aBitmap); aBitmap = nullptr;
  if (nBitmap) free(nBitmap); nBitmap = nullptr;
}