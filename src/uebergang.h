#ifndef __UEBERGANG_H
#define __UEBERGANG_H

#include <Preferences.h>
#include "konfiguration.h"

class Uebergang {
public:
  virtual void prefs_laden(Preferences p);
  virtual void prefs_schreiben(Preferences p);
  virtual String prefs_ausgeben();
  virtual bool doit(struct sKonfiguration *alt, struct sKonfiguration *neu);
};

#endif