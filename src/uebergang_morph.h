#ifndef __UEBERGANG_MORPH_H
#define __UEBERGANG_MORPH_H

#include <Arduino.h>
#include <Preferences.h>
#include "uebergang.h"


class Uebergang_Morph : public Uebergang_sd {
private:
  uint32_t morph_step;

public:
  Uebergang_Morph(bool aktiv, uint16_t gewichtung, uint16_t steps, uint16_t delay);

  bool doit(struct sKonfiguration *alt, struct sKonfiguration *neu) override;
};

#endif