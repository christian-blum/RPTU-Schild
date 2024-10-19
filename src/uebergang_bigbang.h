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
void uebergang_bigbang_prefs_laden(Preferences p);
String uebergang_bigbang_prefs_ausgeben();
void uebergang_bigbang_prefs_schreiben(Preferences p);

class Uebergang_Bigbang : public Uebergang {
public:
  void prefs_laden(Preferences p) override;
  void prefs_schreiben(Preferences p) override;
  String prefs_ausgeben() override;
  bool doit(struct sKonfiguration *alt, struct sKonfiguration *neu) override;
};

#endif