#ifndef __UEBERGANG_H
#define __UEBERGANG_H

#include <Preferences.h>
#include "konfiguration.h"

class Uebergang {
public:
  Uebergang(bool aktiv, uint16_t gewichtung);

  const char* name;
  const char* beschreibung;
  const char* tag;
  bool default_aktiv;
  bool aktiv;
  uint16_t default_gewichtung;
  uint16_t gewichtung;

  virtual void prefs_laden(Preferences& p);
  virtual void prefs_schreiben(Preferences& p);
  virtual void prefs_ausgeben(String& s);
  virtual bool doit(struct sKonfiguration *alt, struct sKonfiguration *neu);
};

#endif