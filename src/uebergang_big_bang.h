#ifndef __UEBERGANG_BIGBANG_H
#define __UEBERGANG_BIGBANG_H

#include <Arduino.h>
#include <Preferences.h>
#include "uebergang.h"

#define BIGBANG_STEPS 40
#define BIGBANG_DELAY 50

extern uint16_t uebergang_bigbang_steps;
extern uint16_t uebergang_bigbang_delay;

bool uebergang_bigbang(struct sKonfiguration *alt, struct sKonfiguration *neu);
void uebergang_bigbang_prefs_laden(Preferences& p);
void uebergang_bigbang_prefs_ausgeben(String& s);
void uebergang_bigbang_prefs_schreiben(Preferences& p);

class Uebergang_Big_Bang : public Uebergang {
public:
  Uebergang_Big_Bang(bool aktiv, uint16_t gewichtung, uint16_t steps, uint16_t delay);

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