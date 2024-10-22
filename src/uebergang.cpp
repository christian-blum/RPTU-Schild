#include <Arduino.h>
#include <Preferences.h>

#include "uebergaenge.h"
#include "uebergang.h"






Uebergang::Uebergang(bool aktiv, uint16_t gewichtung) {
  Uebergang::aktiv = Uebergang::default_aktiv = aktiv;
  Uebergang::gewichtung = Uebergang::default_gewichtung = gewichtung;
  parameter.push_back(P_AKTIV);
  parameter.push_back(P_GEWICHTUNG);
}

Uebergang::~Uebergang() {}

void Uebergang::prefs_laden(Preferences& p) {
  aktiv = p.getBool(PREF_AKTIV, aktiv);
  gewichtung = p.getUShort(PREF_GEWICHTUNG, gewichtung);
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

void Uebergang::prefs_defaults() {
  aktiv = default_aktiv;
  gewichtung = default_gewichtung;
  uebergaenge_gewichtungen_summieren();
}

bool Uebergang::doit(struct sKonfiguration *alt, struct sKonfiguration *neu) {
  log_e("oops!");
  return true;
}



Uebergang_d::Uebergang_d(bool aktiv, uint16_t gewichtung, uint16_t delay) : Uebergang(aktiv, gewichtung) {
  Uebergang_d::delay = default_delay = delay;
  parameter.push_back(P_DELAY);
}

Uebergang_d::~Uebergang_d() {
}

void Uebergang_d::prefs_laden(Preferences& p) {
  Uebergang::prefs_laden(p);
  delay = p.getUShort(PREF_DELAY, delay);
  if (delay > 1000) delay = 1000;
}

void Uebergang_d::prefs_schreiben(Preferences& p) {
  Uebergang::prefs_schreiben(p);
  if (p.getUShort(PREF_DELAY) != delay) p.putUShort(PREF_DELAY, delay);
}

void Uebergang_d::prefs_ausgeben(String& s) {
  Uebergang::prefs_ausgeben(s);
  PREF_AUSGEBEN(s, PREF_DELAY, delay);
}

void Uebergang_d::prefs_defaults() {
  delay = default_delay;
  Uebergang::prefs_defaults();
}

Uebergang_sd::Uebergang_sd(bool aktiv, uint16_t gewichtung, uint16_t steps, uint16_t delay) : Uebergang_d(aktiv, gewichtung, delay) {
  Uebergang_sd::steps = default_steps = steps;
  parameter.push_back(P_STEPS);
}

Uebergang_sd::~Uebergang_sd() {
}

void Uebergang_sd::prefs_laden(Preferences& p) {
  Uebergang::prefs_laden(p);
  steps = p.getUShort(PREF_STEPS, steps);
  if (steps == 0 || steps > 2000) steps = default_steps;
}

void Uebergang_sd::prefs_schreiben(Preferences& p) {
  Uebergang::prefs_schreiben(p);
  if (p.getUShort(PREF_STEPS) != steps) p.putUShort(PREF_STEPS, steps);
}

void Uebergang_sd::prefs_ausgeben(String& s) {
  Uebergang::prefs_ausgeben(s);
  PREF_AUSGEBEN(s, PREF_STEPS, steps);
}

void Uebergang_sd::prefs_defaults() {
  steps = default_steps;
  Uebergang_d::prefs_defaults();
}