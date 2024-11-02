#ifndef __EINSTELLUNGEN_H
#define __EINSTELLUNGEN_H

#include <Arduino.h>
#include "defaults.h"

// Bezeichnungen der Präferenzen in Preferences
#define PREF_EINAUS "einaus"
#define PREF_HELLIGKEIT "helligkeit"
#define PREF_EFFEKTE "effekte"
#define PREF_HG_SCHWARZ "hg_schwarz"
#ifdef HAVE_BLUETOOTH
#define PREF_BT_EINAUS "bteinaus"
#define PREF_BT_DEVICE_NAME "btdn"
#endif


#define PREF_APPEND(s,x,y) s += x; s += " = "; s += y; s += "\n";

extern bool einaus;
extern volatile uint8_t helligkeit;
extern bool effekte_einaus;
extern bool hintergrund_schwarz;
#ifdef HAVE_BLUETOOTH
extern uint8_t bt_einaus;
extern String bt_device_name;
#endif


extern uint32_t effekt_pause_max;
extern uint32_t effekt_pause_min;

extern uint16_t konfiguration_pause;



extern volatile bool preferences_speichern;

extern const char *credits;
extern const char *releaseInfo;

#include "cb_scheduler.h"
extern CB_Scheduler scheduler;

void preferences_ausgeben();
void preferences_laden();
void preferences_schreiben();

#endif