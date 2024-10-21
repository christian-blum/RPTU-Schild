#ifndef __UEBERGANG_EPILLEPTISCHER_ANFALL_H
#define __UEBERGANG_EPILLEPTISCHER_ANFALL_H

#include <Arduino.h>
#include <Preferences.h>

#include "uebergang.h"

#define EPILLEPTISCHER_ANFALL_STEPS 30
#define EPILLEPTISCHER_ANFALL_DELAY 110


bool uebergang_epilleptischer_anfall(struct sKonfiguration *alt, struct sKonfiguration *neu);
void uebergang_epilleptischer_anfall_prefs_laden(Preferences& p);
void uebergang_epilleptischer_anfall_prefs_ausgeben(String& s);
void uebergang_epilleptischer_anfall_prefs_schreiben(Preferences& p);

class Uebergang_Epilleptischer_Anfall : public Uebergang {
public:
  Uebergang_Epilleptischer_Anfall(bool aktiv, uint16_t gewichtung, uint16_t steps, uint16_t delay);

  const char* name = "Epilleptischer Anfall";
  const char* beschreibung = "Hektisches Geflacker mit realer Gefahr für empfindliche Menschen.";
  const char* tag = "ea";

  uint16_t default_steps;
  uint16_t steps;
  uint16_t default_delay;
  uint16_t delay;

  void prefs_laden(Preferences& p) override;
  void prefs_schreiben(Preferences& p) override;
  void prefs_ausgeben(String& s) override;
  bool doit(struct sKonfiguration *alt, struct sKonfiguration *neu) override;
};

#endif