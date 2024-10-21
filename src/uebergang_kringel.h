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
void uebergang_kringel_prefs_laden(Preferences& p);
void uebergang_kringel_prefs_ausgeben(String& p);
void uebergang_kringel_prefs_schreiben(Preferences& p);

class Uebergang_Kringel : public Uebergang {
private:

public:
  Uebergang_Kringel(bool aktiv, uint16_t gewichtung, uint16_t steps, uint16_t delay, int8_t richtung); // +1 oder -1

  const char* name = "Kringel";
  const char* beschreibung = "Buchstaben rotieren mit unterschiedlicher Geschwindigkeit und wechseln dabei gegebenenfalls nach und nach die Umlaufbahn. Rechtsrum oder linksrum.";
  const char* tag = "kringel";

  uint16_t default_steps;
  uint16_t steps;
  uint16_t default_delay;
  uint16_t delay;
  int8_t richtung;

  void prefs_laden(Preferences& p) override;
  void prefs_schreiben(Preferences& p) override;
  void prefs_ausgeben(String& s) override;
  bool doit(struct sKonfiguration *alt, struct sKonfiguration *neu) override;
};

class Uebergang_Kringel_linksrum : public Uebergang_Kringel {
public:
  Uebergang_Kringel_linksrum(bool aktiv, uint16_t gewichtung, uint16_t steps, uint16_t delay);
};

class Uebergang_Kringel_rechtsrum : public Uebergang_Kringel {
public:
  Uebergang_Kringel_rechtsrum(bool aktiv, uint16_t gewichtung, uint16_t steps, uint16_t delay);
};

#endif