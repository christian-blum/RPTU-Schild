#ifndef __UEBERGANG_BLEND_H
#define __UEBERGANG_BLEND_H

#include <Arduino.h>
#include <Preferences.h>
#include "uebergang.h"


class Uebergang_Blend : public Uebergang_sd {
private:
  int blend_step;
  struct sCRGBA *aBitmap;
  struct sCRGBA *nBitmap;

public:
  Uebergang_Blend(bool aktiv, uint16_t gewichtung, uint16_t steps, uint16_t delay);
 ~Uebergang_Blend();
 
  bool doit(struct sKonfiguration *alt, struct sKonfiguration *neu) override;
};

#endif