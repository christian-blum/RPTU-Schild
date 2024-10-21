#include <Arduino.h>

#include "uebergang.h"
#include "uebergang_epilleptischer_anfall.h"
#include "uebergaenge.h"
#include "konfiguration.h"
#include "einstellungen.h"


Uebergang_Epilleptischer_Anfall::Uebergang_Epilleptischer_Anfall(bool aktiv, uint16_t gewichtung, uint16_t steps, uint16_t delay) : Uebergang_sd(aktiv, gewichtung, steps, delay) {
  name = (char *)"Epilleptischer Anfall";
  beschreibung = (char *)"Hektisches Geflacker mit realer Gefahr für empfindliche Menschen.";
  tag = (char *)"ea";
  ea_step = 0;
}

bool Uebergang_Epilleptischer_Anfall::doit(struct sKonfiguration *alt, struct sKonfiguration *neu) {
  if (ea_step == 0) {
    uebergang_queueKonfiguration(alt, delay);
  }
  else if (ea_step >= steps) {
    uebergang_queueKonfiguration(neu, konfiguration_pause);
    ea_step = 0;
    return true;
  }
  struct sKonfiguration k = konfiguration_wuerfeln();
  uebergang_queueKonfiguration(&k, delay);
  ea_step++;
  return false;
}
