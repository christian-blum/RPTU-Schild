#ifndef __UEBERGAENGE_H
#define __UEBERGAENGE_H

#include <Arduino.h>
#include <Preferences.h>

#include "led_matrix.h"
#include "uebergang.h"

struct sPosition morph_position(struct sPosition alt, struct sPosition neu, uint32_t morph_step, uint32_t morph_steps);
struct sCRGBA morph_color(struct sCRGBA alt, struct sCRGBA neu, uint32_t i, uint32_t morph_steps);

extern std::array<Uebergang *, 8> uebergaenge;

void uebergaenge_prefs_laden();
void uebergaenge_prefs_schreiben();
void uebergaenge_prefs_ausgeben(String& s);
void setup_uebergaenge();
void base_pipeline_fuellen();
void uebergaenge_gewichtungen_summieren();

#endif