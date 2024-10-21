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
void uebergang_morph_prefs_laden(Preferences& p);
void uebergang_morph_prefs_ausgeben(String& p);
void uebergang_morph_prefs_schreiben(Preferences& p);

class Uebergang_Morph : public Uebergang {
public:
  Uebergang_Morph(bool aktiv, uint16_t gewichtung, uint16_t steps, uint16_t delay);

  const char* name = "Morph";
  const char* beschreibung = "Buchstaben bewegen sich in ihre neue Position, dabei ändern sich Farben kontinuierlich.";
  const char* tag = "morph";

  uint16_t default_steps;
  uint16_t steps;
  uint16_t default_delay;
  uint16_t delay;

  void prefs_laden(Preferences& p) override;
  void prefs_schreiben(Preferences& p) override;
  void prefs_ausgeben(String& s) override;
  bool doit(struct sKonfiguration *alt, struct sKonfiguration *neu) override;
};

#endif