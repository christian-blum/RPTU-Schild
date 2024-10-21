#include <Arduino.h>
#include <Preferences.h>

#include "uebergang.h"


Uebergang::Uebergang(bool aktiv, uint16_t gewichtung) {
  Uebergang::aktiv = Uebergang::default_aktiv = aktiv;
  Uebergang::gewichtung = Uebergang::default_gewichtung = gewichtung;
}

Uebergang::~Uebergang() {}

void Uebergang::prefs_laden(Preferences& p) {
  aktiv = p.getBool(PREF_AKTIV, aktiv);
  gewichtung = p.getBool(PREF_GEWICHTUNG, gewichtung);
}

void Uebergang::prefs_laden() {
  Preferences p;
  p.begin(tag, true);
  prefs_laden(p);
  p.end();
}

void Uebergang::prefs_schreiben(Preferences& p) {
  if (p.getBool(PREF_AKTIV) != aktiv) p.putBool(PREF_AKTIV, aktiv);
  if (p.getUShort(PREF_GEWICHTUNG) != gewichtung) p.putUShort(PREF_GEWICHTUNG, gewichtung);
}

void Uebergang::prefs_schreiben() {
  Preferences p;
  p.begin(tag, false);
  prefs_schreiben(p);
  p.end();
}

void Uebergang::prefs_ausgeben(String& s) {
  PREF_AUSGEBEN(s, PREF_AKTIV, aktiv);
  PREF_AUSGEBEN(s, PREF_GEWICHTUNG, gewichtung);
}

bool Uebergang::doit(struct sKonfiguration *alt, struct sKonfiguration *neu) {
  log_e("oops!");
  return true;
}

Uebergang_sd::Uebergang_sd(bool aktiv, uint16_t gewichtung, uint16_t steps, uint16_t delay) : Uebergang(aktiv, gewichtung) {
  Uebergang_sd::steps = default_steps = steps;
  Uebergang_sd::delay = default_delay = delay;
}

Uebergang_sd::~Uebergang_sd() {
}

void Uebergang_sd::prefs_laden(Preferences& p) {
  Uebergang::prefs_laden(p);
  steps = p.getUShort(PREF_STEPS, steps);
  delay = p.getUShort(PREF_DELAY, delay);
}

void Uebergang_sd::prefs_schreiben(Preferences& p) {
  Uebergang::prefs_schreiben(p);
  if (p.getUShort(PREF_STEPS) != steps) p.putUShort(PREF_STEPS, steps);
  if (p.getUShort(PREF_DELAY) != delay) p.putUShort(PREF_DELAY, delay);
}

void Uebergang_sd::prefs_ausgeben(String& s) {
  Uebergang::prefs_ausgeben(s);
  PREF_AUSGEBEN(s, PREF_STEPS, steps);
  PREF_AUSGEBEN(s, PREF_DELAY, delay);
}
