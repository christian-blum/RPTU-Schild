#ifndef __UEBERGANG_BIGBANG_H
#define __UEBERGANG_BIGBANG_H

#include <Arduino.h>
#include <Preferences.h>
#include "uebergang.h"


class Uebergang_Big_Bang : public Uebergang_sd {
private:
  uint8_t bigbang_phase;
  struct sKonfiguration bigbang_mitte;

public:
  Uebergang_Big_Bang(bool aktiv, uint16_t gewichtung, uint16_t steps, uint16_t delay);

  bool doit(struct sKonfiguration *alt, struct sKonfiguration *neu) override;
};

#endif