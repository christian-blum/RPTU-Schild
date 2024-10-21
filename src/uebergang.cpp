#include <Arduino.h>
#include <Preferences.h>

#include "uebergang.h"


Uebergang::Uebergang(bool aktiv, uint16_t gewichtung) {
  Uebergang::aktiv = Uebergang::default_aktiv = aktiv;
  Uebergang::gewichtung = Uebergang::default_gewichtung = gewichtung;
}

void Uebergang::prefs_laden(Preferences& p) {}
void Uebergang::prefs_schreiben(Preferences& p) {}
void Uebergang::prefs_ausgeben(String& s) {}
bool Uebergang::doit(struct sKonfiguration *alt, struct sKonfiguration *neu) {
  log_e("oops!");
  return true;
}