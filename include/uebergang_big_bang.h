#ifndef __UEBERGANG_BIGBANG_H
#define __UEBERGANG_BIGBANG_H

#include <Arduino.h>
#include <Preferences.h>
#include "uebergang.h"


class Uebergang_Big_Bang : public Uebergang_sd {
private:
  uint8_t bigbang_phase;
  uint16_t morph_step;
  struct sKonfiguration bigbang_mitte;

  bool uebergang_morph(struct sKonfiguration *alt, struct sKonfiguration *neu, uint16_t steps, uint16_t delay, uint16_t delay_end);

public:
  Uebergang_Big_Bang(bool aktiv, uint16_t gewichtung, uint16_t steps, uint16_t delay);

  bool doit(struct sKonfiguration *alt, struct sKonfiguration *neu) override;
};

#endif