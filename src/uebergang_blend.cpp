#include <Arduino.h>

#include "uebergaenge.h"
#include "uebergang_blend.h"
#include "led_matrix.h"
#include "konfiguration.h"
#include "einstellungen.h"

static int blend_step;
static struct sCRGBA *aBitmap;
static struct sCRGBA *nBitmap;

uint16_t uebergang_blend_steps;
uint16_t uebergang_blend_delay;

#define PREF_BLEND_STEPS "blend_steps"
#define PREF_BLEND_DELAY "blend_delay"


void uebergang_blend_prefs_laden(Preferences p) {
  uebergang_blend_steps = p.getUShort(PREF_BLEND_STEPS, BLEND_STEPS);
  uebergang_blend_delay = p.getUShort(PREF_BLEND_DELAY, BLEND_DELAY);
}

String uebergang_blend_prefs_ausgeben() {
  return PREF_TO_STRING(PREF_BLEND_STEPS, uebergang_blend_steps) + PREF_TO_STRING(PREF_BLEND_DELAY, uebergang_blend_delay);
}

void uebergang_blend_prefs_schreiben(Preferences p) {
  if (p.getUShort(PREF_BLEND_STEPS) != uebergang_blend_steps) p.putUShort(PREF_BLEND_STEPS, uebergang_blend_steps);
  if (p.getUShort(PREF_BLEND_DELAY) != uebergang_blend_delay) p.putUShort(PREF_BLEND_DELAY, uebergang_blend_delay);
}



// alte Konfiguration langsam in neue Konfiguration überblenden
bool uebergang_blend(struct sKonfiguration *alt, struct sKonfiguration *neu) {
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
    bitmap[p] = morph_color(aBitmap[p], nBitmap[p], blend_step, uebergang_blend_steps);
  }
  blend_step++;
  b->milliseconds = (blend_step == uebergang_blend_steps + 1) ? konfiguration_pause : uebergang_blend_delay;
  base_queue(b);

  if (blend_step >= uebergang_blend_steps + 1) {
    free(aBitmap);
    free(nBitmap);
    blend_step = 0;
    return true;
  }
  return false;
}
