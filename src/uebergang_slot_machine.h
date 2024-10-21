#ifndef __UEBERGANG_SLOT_MACHINE_H
#define __UEBERGANG_SLOT_MACHINE_H

#include <Arduino.h>
#include <Preferences.h>
#include "uebergang.h"

#define PREF_SPEED_MIN "speed_min"
#define PREF_SPEED_MAX "speed_max"
#define PREF_DAMP_MIN "damp_min"
#define PREF_DAMP_MAX "damp_max"

class Uebergang_Slot_Machine : public Uebergang_sd {
private:
  uint8_t slot_woWirSind;
  float slot_positions[4];
  float slot_speeds[4];
  float slot_dampening[4];
  struct sKonfiguration * slot_letzteKonfiguration;
  uint32_t slot_gesamt;
  uint32_t slot_count;
  uint8_t morph_step;

  bool uebergang_morph(struct sKonfiguration *alt, struct sKonfiguration *neu, uint16_t morph_steps, uint16_t morph_delay, uint16_t morph_delay_ende);

public:
  Uebergang_Slot_Machine(bool aktiv, uint16_t gewichtung, uint16_t delay, float speed_min, float speed_max, float damp_min, float damp_max);
 ~Uebergang_Slot_Machine();
 
  float default_speed_min;
  float speed_min;
  float default_speed_max;
  float speed_max;
  float default_damp_min;
  float damp_min;
  float default_damp_max;
  float damp_max;

  void prefs_laden(Preferences& p) override;
  void prefs_schreiben(Preferences& p) override;
  void prefs_ausgeben(String& s) override;
  bool doit(struct sKonfiguration *alt, struct sKonfiguration *neu) override;
};

#endif