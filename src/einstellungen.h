#ifndef __EINSTELLUNGEN_H
#define __EINSTELLUNGEN_H

#include <Arduino.h>

#define PREF_TO_STRING(x,y) String(x) + " = " + String(y) + "\n"

extern bool einaus;
extern volatile uint8_t helligkeit;
extern bool effekte_einaus;
extern bool hintergrund_schwarz;
#ifdef HAVE_BLUETOOTH
extern uint8_t bt_einaus;
extern String bt_device_name;
#endif

extern uint16_t uebergang_bigbang_steps;
extern uint16_t uebergang_bigbang_delay;
extern uint16_t uebergang_blend_steps;
extern uint16_t uebergang_blend_delay;
extern uint16_t uebergang_kringel_steps;
extern uint16_t uebergang_kringel_delay;
extern uint16_t uebergang_ea_steps;
extern uint16_t uebergang_ea_delay;
extern float uebergang_slot_speed_max;
extern float uebergang_slot_speed_min;
extern float uebergang_slot_damp_max;
extern float uebergang_slot_damp_min;
extern uint16_t uebergang_slot_delay;

extern uint32_t effekt_pause_max;
extern uint32_t effekt_pause_min;
extern uint16_t laufschrift_delay;

extern uint16_t konfiguration_pause;



extern volatile bool preferences_speichern;

extern const char *credits;
extern const char *releaseInfo;

void preferences_ausgeben();
void preferences_laden();
void preferences_schreiben();
void preferences_loeschen();

#endif