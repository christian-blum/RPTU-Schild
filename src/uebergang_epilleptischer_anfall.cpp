#include <Arduino.h>

#include "uebergang.h"
#include "uebergang_epilleptischer_anfall.h"
#include "uebergaenge.h"
#include "konfiguration.h"
#include "einstellungen.h"

#define PREF_EA_STEPS "ea_steps"
#define PREF_EA_DELAY "ea_delay"

uint16_t uebergang_ea_steps;
uint16_t uebergang_ea_delay;

uint8_t ea_step;


void uebergang_epilleptischer_anfall_prefs_laden(Preferences& p) {
  uebergang_ea_steps = p.getUShort(PREF_EA_STEPS, EPILLEPTISCHER_ANFALL_STEPS);
  uebergang_ea_delay = p.getUShort(PREF_EA_DELAY, EPILLEPTISCHER_ANFALL_DELAY);
}

void uebergang_epilleptischer_anfall_prefs_ausgeben(String& s) {
  PREF_APPEND(s, PREF_EA_STEPS, uebergang_ea_steps);
  PREF_APPEND(s, PREF_EA_DELAY, uebergang_ea_delay);
}

void uebergang_epilleptischer_anfall_prefs_schreiben(Preferences& p) {
  if (p.getUShort(PREF_EA_STEPS) != uebergang_ea_steps) p.putUShort(PREF_EA_STEPS, uebergang_ea_steps);
  if (p.getUShort(PREF_EA_DELAY) != uebergang_ea_delay) p.putUShort(PREF_EA_DELAY, uebergang_ea_delay);
}


bool uebergang_epilleptischer_anfall(struct sKonfiguration *alt, struct sKonfiguration *neu) {
  if (ea_step == 0) {
    uebergang_queueKonfiguration(alt, uebergang_ea_delay);
  }
  else if (ea_step >= uebergang_ea_steps) {
    uebergang_queueKonfiguration(neu, konfiguration_pause);
    ea_step = 0;
    return true;
  }
  struct sKonfiguration k = konfiguration_wuerfeln();
  uebergang_queueKonfiguration(&k, uebergang_ea_delay);
  ea_step++;
  return false;
}




Uebergang_Epilleptischer_Anfall::Uebergang_Epilleptischer_Anfall(bool aktiv, uint16_t gewichtung, uint16_t steps, uint16_t delay) : Uebergang(aktiv, gewichtung) {
  Uebergang_Epilleptischer_Anfall::steps = default_steps = steps;
  Uebergang_Epilleptischer_Anfall::delay = default_delay = delay;
  name = (char *)"Epilleptischer Anfall";
  beschreibung = (char *)"Hektisches Geflacker mit realer Gefahr für empfindliche Menschen.";
  tag = (char *)"ea";
}

void Uebergang_Epilleptischer_Anfall::prefs_laden(Preferences& p) {
  uebergang_epilleptischer_anfall_prefs_laden(p);
}

void Uebergang_Epilleptischer_Anfall::prefs_schreiben(Preferences& p) {
  uebergang_epilleptischer_anfall_prefs_schreiben(p);
}

void Uebergang_Epilleptischer_Anfall::prefs_ausgeben(String& s) {
  uebergang_epilleptischer_anfall_prefs_ausgeben(s);
}

bool Uebergang_Epilleptischer_Anfall::doit(struct sKonfiguration *alt, struct sKonfiguration *neu) {
  return uebergang_epilleptischer_anfall(alt, neu);
}
