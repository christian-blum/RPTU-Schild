#ifndef __UEBERGANG_MORPH_H
#define __UEBERGANG_MORPH_H

#include <Arduino.h>
#include <Preferences.h>

#define MORPH_STEPS 40
#define MORPH_DELAY 50

extern uint16_t uebergang_morph_steps;
extern uint16_t uebergang_morph_delay;

bool uebergang_morph(struct sKonfiguration *alt, struct sKonfiguration *neu, uint16_t morph_steps, uint16_t morph_delay, uint16_t morph_delay_ende);
bool uebergang_morph(struct sKonfiguration *alt, struct sKonfiguration *neu);
void uebergang_morph_prefs_laden(Preferences p);
String uebergang_morph_prefs_ausgeben();
void uebergang_morph_prefs_schreiben(Preferences p);

#endif