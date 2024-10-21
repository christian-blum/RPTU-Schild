#include <Arduino.h>

#include "uebergang_big_bang.h"
#include "einstellungen.h"
#include "led_matrix.h"
#include "uebergaenge.h"
#include "uebergang_morph.h"
#include "konfiguration.h"


uint16_t uebergang_bigbang_steps;
uint16_t uebergang_bigbang_delay;

static uint8_t bigbang_phase;
static struct sKonfiguration bigbang_mitte;

#define PREF_BIGBANG_STEPS "bigbang_steps"
#define PREF_BIGBANG_DELAY "bigbang_delay"


void uebergang_bigbang_prefs_laden(Preferences& p) {
  uebergang_bigbang_steps = p.getUShort(PREF_BIGBANG_STEPS, BIGBANG_STEPS);
  uebergang_bigbang_delay = p.getUShort(PREF_BIGBANG_DELAY, BIGBANG_DELAY);
}

void uebergang_bigbang_prefs_ausgeben(String& s) {
  PREF_APPEND(s, PREF_BIGBANG_STEPS, uebergang_bigbang_steps);
  PREF_APPEND(s, PREF_BIGBANG_DELAY, uebergang_bigbang_delay);
}

void uebergang_bigbang_prefs_schreiben(Preferences& p) {
  if (p.getUShort(PREF_BIGBANG_STEPS) != uebergang_bigbang_steps) p.putUShort(PREF_BIGBANG_STEPS, uebergang_bigbang_steps);
  if (p.getUShort(PREF_BIGBANG_DELAY) != uebergang_bigbang_delay) p.putUShort(PREF_BIGBANG_DELAY, uebergang_bigbang_delay);
}


bool uebergang_bigbang(struct sKonfiguration *alt, struct sKonfiguration *neu) {
  bool fertig;
  switch (bigbang_phase) {
  case 0:
    bigbang_mitte.r.x = (LED_COUNT_X - 8) / 2; bigbang_mitte.r.y = (LED_COUNT_Y - 8) / 2;
    bigbang_mitte.p.x = (LED_COUNT_X - 8) / 2; bigbang_mitte.p.y = (LED_COUNT_Y - 8) / 2;
    bigbang_mitte.t.x = (LED_COUNT_X - 8) / 2; bigbang_mitte.t.y = (LED_COUNT_Y - 8) / 2;
    bigbang_mitte.u.x = (LED_COUNT_X - 8) / 2; bigbang_mitte.u.y = (LED_COUNT_Y - 8) / 2;
    bigbang_mitte.schriftfarbe = morph_color(alt->schriftfarbe, neu->schriftfarbe, 1, 2);
    bigbang_mitte.hintergrundfarbe = morph_color(alt->hintergrundfarbe, neu->hintergrundfarbe, 1, 2);
    bigbang_phase = 1;
    return false;
  case 1:
    fertig = uebergang_morph(alt, &bigbang_mitte, uebergang_bigbang_steps/2, uebergang_bigbang_delay, uebergang_bigbang_delay);
    if (fertig) bigbang_phase = 2;
    return false;
  case 2:
    fertig = uebergang_morph(&bigbang_mitte, neu, uebergang_bigbang_steps/2, uebergang_bigbang_delay, konfiguration_pause);
    if (fertig) bigbang_phase = 3;
    return false;
  default:
    bigbang_phase = 0;
    return true;
  }
}


Uebergang_Big_Bang::Uebergang_Big_Bang(bool aktiv, uint16_t gewichtung, uint16_t steps, uint16_t delay) : Uebergang(aktiv, gewichtung) {
  Uebergang_Big_Bang::steps = default_steps = steps;
  Uebergang_Big_Bang::delay = default_delay = delay;
}

void Uebergang_Big_Bang::prefs_laden(Preferences& p) {
  uebergang_bigbang_prefs_laden(p);
}

void Uebergang_Big_Bang::prefs_schreiben(Preferences& p) {
  uebergang_bigbang_prefs_schreiben(p);
}

void Uebergang_Big_Bang::prefs_ausgeben(String& p) {
  uebergang_bigbang_prefs_ausgeben(p);
}

bool Uebergang_Big_Bang::doit(struct sKonfiguration *alt, struct sKonfiguration *neu) {
  return uebergang_bigbang(alt, neu);
}
