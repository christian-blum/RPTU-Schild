#include <Arduino.h>
#include <Preferences.h>

#include "uebergang.h"
#include "uebergang_morph.h"
#include "einstellungen.h"
#include "led_matrix.h"
#include "konfiguration.h"
#include "uebergaenge.h"

#define PREF_MORPH_STEPS "morph_steps"
#define PREF_MORPH_DELAY "morph_delay"

uint16_t uebergang_morph_steps;
uint16_t uebergang_morph_delay;

void uebergang_morph_prefs_laden(Preferences p) {
  uebergang_morph_steps = p.getUShort(PREF_MORPH_STEPS, MORPH_STEPS);
  uebergang_morph_delay = p.getUShort(PREF_MORPH_DELAY, MORPH_DELAY);
}

String uebergang_morph_prefs_ausgeben() {
  return PREF_TO_STRING(PREF_MORPH_STEPS, uebergang_morph_steps) + PREF_TO_STRING(PREF_MORPH_DELAY, uebergang_morph_delay);
}

void uebergang_morph_prefs_schreiben(Preferences p) {
  if (p.getUShort(PREF_MORPH_STEPS) != uebergang_morph_steps) p.putUShort(PREF_MORPH_STEPS, uebergang_morph_steps);
  if (p.getUShort(PREF_MORPH_DELAY) != uebergang_morph_delay) p.putUShort(PREF_MORPH_DELAY, uebergang_morph_delay);
}

static uint32_t morph_step;

bool uebergang_morph(struct sKonfiguration *alt, struct sKonfiguration *neu, uint16_t morph_steps, uint16_t morph_delay, uint16_t morph_delay_ende) {
  struct sKonfiguration x;
  x.r = morph_position(alt->r, neu->r, morph_step, morph_steps);
  x.p = morph_position(alt->p, neu->p, morph_step, morph_steps);
  x.t = morph_position(alt->t, neu->t, morph_step, morph_steps);
  x.u = morph_position(alt->u, neu->u, morph_step, morph_steps);
  x.schriftfarbe = morph_color(alt->schriftfarbe, neu->schriftfarbe, morph_step, morph_steps);
  x.hintergrundfarbe = morph_color(alt->hintergrundfarbe, neu->hintergrundfarbe, morph_step, morph_steps);

  morph_step++;
  uebergang_queueKonfiguration(&x, ((morph_step == morph_steps + 1) && (morph_steps>3)) ? morph_delay_ende : morph_delay);
  
  if (morph_step >= morph_steps + 1) {
    morph_step = 0;
    return true;
  }
  return false;
}


bool uebergang_morph(struct sKonfiguration *alt, struct sKonfiguration *neu) {
  return uebergang_morph(alt, neu, uebergang_morph_steps, uebergang_morph_delay, konfiguration_pause);
}


void Uebergang_Morph::prefs_laden(Preferences p) {
  uebergang_morph_prefs_laden(p);
}

void Uebergang_Morph::prefs_schreiben(Preferences p) {
  uebergang_morph_prefs_schreiben(p);
}

String Uebergang_Morph::prefs_ausgeben() {
  return uebergang_morph_prefs_ausgeben();
}

bool Uebergang_Morph::doit(struct sKonfiguration *alt, struct sKonfiguration *neu) {
  return uebergang_morph(alt, neu);
}
