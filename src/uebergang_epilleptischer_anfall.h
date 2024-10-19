#ifndef __UEBERGANG_EPILLEPTISCHER_ANFALL_H
#define __UEBERGANG_EPILLEPTISCHER_ANFALL_H

#include <Arduino.h>
#include <Preferences.h>

#define EPILLEPTISCHER_ANFALL_STEPS 30
#define EPILLEPTISCHER_ANFALL_DELAY 110


bool uebergang_epilleptischer_anfall(struct sKonfiguration *alt, struct sKonfiguration *neu);
void uebergang_epilleptischer_anfall_prefs_laden(Preferences p);
String uebergang_epilleptischer_anfall_prefs_ausgeben();
void uebergang_epilleptischer_anfall_prefs_schreiben(Preferences p);

#endif