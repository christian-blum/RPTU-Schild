#include <Arduino.h>
#include <Preferences.h>

#include "uebergang.h"
#include "uebergang_jump.h"
#include "einstellungen.h"


// Ein sehr simpler Übergang, der nur aus einem Schritt besteht: er setzt die neue Konfiguration, und fertig.
bool uebergang_jump(struct sKonfiguration *alteKonfiguration, struct sKonfiguration *neueKonfiguration) {
  uebergang_queueKonfiguration(neueKonfiguration, konfiguration_pause);
  return true;
}


Uebergang_Jump::Uebergang_Jump(bool aktiv, uint16_t gewichtung) : Uebergang(aktiv, gewichtung) {
  name = (char *)"Jump";
  beschreibung = (char *)"Simpler Sprung von einer Darstellung zur anderen.";
  tag = (char *)"jump";
}

void Uebergang_Jump::prefs_laden(Preferences& p) {
}

void Uebergang_Jump::prefs_schreiben(Preferences& p) {
}

void Uebergang_Jump::prefs_ausgeben(String& s) {
}

bool Uebergang_Jump::doit(struct sKonfiguration *alt, struct sKonfiguration *neu) {
  return uebergang_jump(alt, neu);
}
