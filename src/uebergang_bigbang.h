#ifndef __UEBERGANG_BIGBANG_H
#define __UEBERGANG_BIGBANG_H

#include <Arduino.h>
#include <Preferences.h>

#define BIGBANG_STEPS 40
#define BIGBANG_DELAY 50

extern uint16_t uebergang_bigbang_steps;
extern uint16_t uebergang_bigbang_delay;

bool uebergang_bigbang(struct sKonfiguration *alt, struct sKonfiguration *neu);
void uebergang_bigbang_prefs_laden(Preferences p);
String uebergang_bigbang_prefs_ausgeben();
void uebergang_bigbang_prefs_schreiben(Preferences p);

#endif