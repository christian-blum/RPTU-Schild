#ifndef __UEBERGANG_KRINGEL_H
#define __UEBERGANG_KRINGEL_H

#include <Arduino.h>
#include <Preferences.h>

#include "uebergang.h"

#define KRINGEL_STEPS 90
#define KRINGEL_DELAY 40

extern uint16_t uebergang_kringel_steps;
extern uint16_t uebergang_kringel_delay;

bool uebergang_kringel(struct sKonfiguration *alt, struct sKonfiguration *neu, int8_t winkelvorzeichen);
bool uebergang_kringel_rechtsrum(struct sKonfiguration *alt, struct sKonfiguration *neu);
bool uebergang_kringel_linksrum(struct sKonfiguration *alt, struct sKonfiguration *neu);
void uebergang_kringel_prefs_laden(Preferences p);
void uebergang_kringel_prefs_ausgeben(String& p);
void uebergang_kringel_prefs_schreiben(Preferences p);

class Uebergang_Kringel : public Uebergang {
private:
  int8_t winkelvorzeichen;

public:
  Uebergang_Kringel(int8_t richtung); // +1 oder -1

  void prefs_laden(Preferences p) override;
  void prefs_schreiben(Preferences p) override;
  void prefs_ausgeben(String& p) override;
  bool doit(struct sKonfiguration *alt, struct sKonfiguration *neu) override;
};

#endif