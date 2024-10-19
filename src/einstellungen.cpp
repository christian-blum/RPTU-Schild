#include <Arduino.h>
#include <Preferences.h>

#include "einstellungen.h"
#include "defaults.h"

const char *credits = "      Gebaut von Tilman Pfersdorff und Christian Blum, 2024      ";
const char *releaseInfo = "      SW Rel. " SOFTWARE_RELEASE "      ";


/***********************************************************
 *                                                         *
 * Preferences                                             *
 *                                                         *
 ***********************************************************/



#define PREF_EFFEKT_PAUSE_MAX "ep_max"
#define PREF_EFFEKT_PAUSE_MIN "ep_min"
#define PREF_LAUFSCHRIFT_DELAY "ls_delay"
#define PREF_KONFIGURATION_PAUSE "konf_pause"

bool einaus;
volatile uint8_t helligkeit;
bool effekte_einaus;
bool hintergrund_schwarz;

uint32_t effekt_pause_max;
uint32_t effekt_pause_min;
uint16_t laufschrift_delay;
uint16_t konfiguration_pause;

volatile bool preferences_speichern;

#define PREF_NAMESPACE_ALLGEMEIN "allgemein"
#define PREF_NAMESPACE_UEBERGAENGE "uebergaenge"

#include "uebergaenge.h"

void preferences_ausgeben() {
  String p = String();
  p += PREF_TO_STRING(PREF_EFFEKTE, effekte_einaus);
  p += PREF_TO_STRING(PREF_HG_SCHWARZ, hintergrund_schwarz);
  p += uebergaenge_prefs_ausgeben();
  p += PREF_TO_STRING(PREF_EFFEKT_PAUSE_MAX, effekt_pause_max);
  p += PREF_TO_STRING(PREF_EFFEKT_PAUSE_MIN, effekt_pause_min);
  p += PREF_TO_STRING(PREF_LAUFSCHRIFT_DELAY, laufschrift_delay);
  p += PREF_TO_STRING(PREF_KONFIGURATION_PAUSE, konfiguration_pause);
  Serial.print(p);
}

void preferences_laden() {
  Preferences p;
  p.begin(PREF_NAMESPACE_ALLGEMEIN, true);
  einaus = p.getBool(PREF_EINAUS, true);
  helligkeit = p.getUChar(PREF_HELLIGKEIT, HELLIGKEIT_DEFAULT);
  effekte_einaus = p.getBool(PREF_EFFEKTE, true);
  hintergrund_schwarz = p.getBool(PREF_HG_SCHWARZ, false);
  uebergaenge_prefs_laden(p);
  effekt_pause_max = p.getULong(PREF_EFFEKT_PAUSE_MAX, EFFEKT_PAUSE_MAX);
  effekt_pause_min = p.getULong(PREF_EFFEKT_PAUSE_MIN, EFFEKT_PAUSE_MIN);
  laufschrift_delay = p.getUShort(PREF_LAUFSCHRIFT_DELAY, LAUFSCHRIFT_DELAY);
  konfiguration_pause = p.getUShort(PREF_KONFIGURATION_PAUSE, KONFIGURATION_PAUSE);

#ifdef HAVE_BLUETOOTH
  bt_einaus = p.getUChar(PREF_BT_EINAUS);
  bt_device_name = p.getString(PREF_BT_DEVICE_NAME, BT_DEFAULT_DEVICE_NAME);
#endif
  p.end();
  preferences_ausgeben();
}

void preferences_schreiben() {
  Preferences p;
  p.begin(PREF_NAMESPACE_ALLGEMEIN, false);
  if (p.getBool(PREF_EINAUS) != einaus) p.putBool(PREF_EINAUS, einaus);
  if (p.getUChar(PREF_HELLIGKEIT) != helligkeit) p.putUChar(PREF_HELLIGKEIT, helligkeit);
  if (p.getBool(PREF_EFFEKTE) != effekte_einaus) p.putBool(PREF_EFFEKTE, effekte_einaus);
  if (p.getBool(PREF_HG_SCHWARZ) != hintergrund_schwarz) p.putBool(PREF_HG_SCHWARZ, hintergrund_schwarz);
  uebergaenge_prefs_schreiben(p);
  if (p.getULong(PREF_EFFEKT_PAUSE_MAX) != effekt_pause_max) p.putULong(PREF_EFFEKT_PAUSE_MAX, effekt_pause_max);
  if (p.getULong(PREF_EFFEKT_PAUSE_MIN) != effekt_pause_min) p.putULong(PREF_EFFEKT_PAUSE_MIN, effekt_pause_min);
  if (p.getUShort(PREF_LAUFSCHRIFT_DELAY) != laufschrift_delay) p.putUShort(PREF_LAUFSCHRIFT_DELAY, laufschrift_delay);
  if (p.getUShort(PREF_KONFIGURATION_PAUSE) != konfiguration_pause) p.putUShort(PREF_KONFIGURATION_PAUSE, konfiguration_pause);

#ifdef HAVE_BLUETOOTH
  if (p.getUChar(PREF_BT_EINAUS) != bt_einaus) p.putUChar(PREF_BT_EINAUS, bt_einaus);
  if (p.getString(PREF_BT_DEVICE_NAME) != bt_device_name) p.putString(PREF_BT_DEVICE_NAME, bt_device_name); // Äh... geht das? Wenn nicht, ggf Abfrage weglassen
#endif
  p.end();
}


void preferences_loeschen() {
  Preferences p;
  p.begin(PREF_NAMESPACE_ALLGEMEIN, false);
  p.clear();
  p.end();
}


