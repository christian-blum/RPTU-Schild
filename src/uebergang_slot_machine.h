#ifndef __UEBERGANG_SLOT_MACHINE_H
#define __UEBERGANG_SLOT_MACHINE_H

#include <Arduino.h>
#include <Preferences.h>

#define SLOT_SPEED_MAX 6.0f
#define SLOT_SPEED_MIN 0.2f
#define SLOT_DAMP_MAX 0.03f
#define SLOT_DAMP_MIN 0.02f
#define SLOT_DELAY 50

extern float uebergang_slot_speed_max;
extern float uebergang_slot_speed_min;
extern float uebergang_slot_damp_max;
extern float uebergang_slot_damp_min;
extern uint16_t uebergang_slot_delay;

bool uebergang_slot_machine(struct sKonfiguration *alt, struct sKonfiguration *neu);
void uebergang_slot_machine_prefs_laden(Preferences& p);
void uebergang_slot_machine_prefs_ausgeben(String& s);
void uebergang_slot_machine_prefs_schreiben(Preferences& p);

class Uebergang_Slot_Machine : public Uebergang {
public:
  using Uebergang::Uebergang;

  const char* name = "Slot Machine";
  const char* beschreibung = "Willkommen in Las Vegas, nur ohne Münzschlitz!";
  const char* tag = "sm";

  void prefs_laden(Preferences& p) override;
  void prefs_schreiben(Preferences& p) override;
  void prefs_ausgeben(String& s) override;
  bool doit(struct sKonfiguration *alt, struct sKonfiguration *neu) override;
};

#endif