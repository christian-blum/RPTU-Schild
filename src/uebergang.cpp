#include <Arduino.h>
#include <Preferences.h>

#include "uebergang.h"

void Uebergang::prefs_laden(Preferences p) {}
void Uebergang::prefs_schreiben(Preferences p) {}
void Uebergang::prefs_ausgeben(String &) {}
bool Uebergang::doit(struct sKonfiguration *alt, struct sKonfiguration *neu) { return true; }