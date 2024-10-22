#ifndef __UEBERGANG_H
#define __UEBERGANG_H

#include <Arduino.h>
#include <Preferences.h>
#include "konfiguration.h"
#include <vector>

// Die brauchen wir für viele Übergänge, und so müssen wir sie nur einmal definieren.
#define PREF_AKTIV "aktiv"
#define PREF_GEWICHTUNG "gewichtung"
#define PREF_STEPS "steps"
#define PREF_DELAY "delay"

#define PREF_AUSGEBEN(s, t, v) s += tag; s += "."; s += t; s += "="; s += v; s += "\n"


enum eUebergangParameterTyp {
  UPT_BOOL,
  UPT_USHORT,
  UPT_FLOAT
};

struct sUebergangParameter {
  const char *tag;
  const char *name;
  const enum eUebergangParameterTyp typ;
  const uint8_t laenge;
  const char *einheit;
  const void *variable;
};


class Uebergang {
private:
  const struct sUebergangParameter P_AKTIV = { "aktiv", "Aktiv", UPT_BOOL, 0, nullptr, (void *) &aktiv };
  const struct sUebergangParameter P_GEWICHTUNG = { "gewichtung", "Gewichtung", UPT_USHORT, 5, nullptr, (void *) &gewichtung };

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

  std::vector<sUebergangParameter> parameter;

  virtual void prefs_laden(Preferences& p);
  virtual void prefs_laden();
  virtual void prefs_schreiben(Preferences& p);
  virtual void prefs_schreiben();
  virtual void prefs_ausgeben(String& s);
  virtual void prefs_defaults();
  virtual bool doit(struct sKonfiguration *alt, struct sKonfiguration *neu);
};


class Uebergang_d : public Uebergang {
private:
  const struct sUebergangParameter P_DELAY = { "delay", "Verzögerung", UPT_USHORT, 5, "ms", (void *) &delay };

public:
  Uebergang_d(bool aktiv, uint16_t gewichtung, uint16_t delay);
 ~Uebergang_d();

  uint16_t default_delay;
  uint16_t delay;

  virtual void prefs_laden(Preferences& p) override;
  virtual void prefs_schreiben(Preferences& p) override;
  virtual void prefs_ausgeben(String& s) override;
  virtual void prefs_defaults() override;
};


class Uebergang_sd : public Uebergang_d {
private:
  const struct sUebergangParameter P_STEPS = { "steps", "Schritte", UPT_USHORT, 5, nullptr, (void *) &steps };

public:
  Uebergang_sd(bool aktiv, uint16_t gewichtung, uint16_t steps, uint16_t delay);
 ~Uebergang_sd();

  uint16_t default_steps;
  uint16_t steps;

  virtual void prefs_laden(Preferences& p) override;
  virtual void prefs_schreiben(Preferences& p) override;
  virtual void prefs_ausgeben(String& s) override;
  virtual void prefs_defaults() override;
};

#endif