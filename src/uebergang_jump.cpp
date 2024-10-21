#include <Arduino.h>
#include <Preferences.h>

#include "uebergang.h"
#include "uebergang_jump.h"
#include "einstellungen.h"



Uebergang_Jump::Uebergang_Jump(bool aktiv, uint16_t gewichtung) : Uebergang(aktiv, gewichtung) {
  name = (char *)"Jump";
  beschreibung = (char *)"Simpler Sprung von einer Darstellung zur anderen.";
  tag = (char *)"jump";
}

bool Uebergang_Jump::doit(struct sKonfiguration *alt, struct sKonfiguration *neu) {
  uebergang_queueKonfiguration(neu, konfiguration_pause);
  return true;
}
