#ifndef __UEBERGAENGE_H
#define __UEBERGAENGE_H

#include <Arduino.h>
#include <Preferences.h>

#include "led_matrix.h"

struct sPosition morph_position(struct sPosition alt, struct sPosition neu, uint32_t morph_step, uint32_t morph_steps);
struct sCRGBA morph_color(struct sCRGBA alt, struct sCRGBA neu, uint32_t i, uint32_t morph_steps);

void uebergaenge_prefs_laden(Preferences& p);
void uebergaenge_prefs_schreiben(Preferences& p);
void uebergaenge_prefs_ausgeben(String& s);
void uebergaenge_laden();
void uebergaenge_speichern();
void setup_uebergaenge();
bool (*wuerfele_uebergang_alt())(struct sKonfiguration *alt, struct sKonfiguration *neu);
void base_pipeline_fuellen();

#endif