#ifndef __UEBERGANG_H
#define __UEBERGANG_H

#include <Arduino.h>
#include <Preferences.h>
#include "konfiguration.h"

// Die brauchen wir für viele Übergänge, und so müssen wir sie nur einmal definieren.
#define PREF_AKTIV "aktiv"
#define PREF_GEWICHTUNG "gewichtung"
#define PREF_STEPS "steps"
#define PREF_DELAY "delay"

#define PREF_AUSGEBEN(s, t, v) s += tag; s += "."; s += t; s += "="; s += v; s += "\n"

class Uebergang {
public:
  Uebergang(bool aktiv, uint16_t gewichtung);
 ~Uebergang();

  char* name = (char *)"parent";
  char* beschreibung = (char *)"parent";
  char* tag = (char *)"parent";
  bool default_aktiv;
  bool aktiv;
  uint16_t default_gewichtung;
  uint16_t gewichtung;

  virtual void prefs_laden(Preferences& p);
  virtual void prefs_laden();
  virtual void prefs_schreiben(Preferences& p);
  virtual void prefs_schreiben();
  virtual void prefs_ausgeben(String& s);
  virtual bool doit(struct sKonfiguration *alt, struct sKonfiguration *neu);
};

class Uebergang_sd : public Uebergang {
public:
  Uebergang_sd(bool aktiv, uint16_t gewichtung, uint16_t steps, uint16_t delay);
 ~Uebergang_sd();

  uint16_t default_steps;
  uint16_t steps;
  uint16_t default_delay;
  uint16_t delay;

  virtual void prefs_laden(Preferences& p) override;
  virtual void prefs_schreiben(Preferences& p) override;
  virtual void prefs_ausgeben(String& s) override;
};

#endif