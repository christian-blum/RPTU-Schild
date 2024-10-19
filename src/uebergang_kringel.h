#ifndef __UEBERGANG_KRINGEL_H
#define __UEBERGANG_KRINGEL_H

#include <Arduino.h>
#include <Preferences.h>

#define KRINGEL_STEPS 90
#define KRINGEL_DELAY 40

extern uint16_t uebergang_kringel_steps;
extern uint16_t uebergang_kringel_delay;

bool uebergang_kringel(struct sKonfiguration *alt, struct sKonfiguration *neu, int8_t winkelvorzeichen);
bool uebergang_kringel_rechtsrum(struct sKonfiguration *alt, struct sKonfiguration *neu);
bool uebergang_kringel_linksrum(struct sKonfiguration *alt, struct sKonfiguration *neu);
void uebergang_kringel_prefs_laden(Preferences p);
String uebergang_kringel_prefs_ausgeben();
void uebergang_kringel_prefs_schreiben(Preferences p);

#endif