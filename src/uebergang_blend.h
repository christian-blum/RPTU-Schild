#ifndef __UEBERGANG_BLEND_H
#define __UEBERGANG_BLEND_H

#include <Arduino.h>
#include <Preferences.h>

#include "uebergang.h"

#define BLEND_STEPS 20
#define BLEND_DELAY 50

extern uint16_t uebergang_blend_steps;
extern uint16_t uebergang_blend_delay;

bool uebergang_blend(struct sKonfiguration *alt, struct sKonfiguration *neu);
void uebergang_blend_prefs_laden(Preferences p);
String uebergang_blend_prefs_ausgeben();
void uebergang_blend_prefs_schreiben(Preferences p);

class Uebergang_Blend : public Uebergang {
public:

  void prefs_laden(Preferences p) override;
  void prefs_schreiben(Preferences p) override;
  String prefs_ausgeben() override;
  bool doit(struct sKonfiguration *alt, struct sKonfiguration *neu) override;
};

#endif