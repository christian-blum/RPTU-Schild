#ifndef __UEBERGANG_EPILLEPTISCHER_ANFALL_H
#define __UEBERGANG_EPILLEPTISCHER_ANFALL_H

#include <Arduino.h>
#include <Preferences.h>
#include "uebergang.h"



class Uebergang_Epilleptischer_Anfall : public Uebergang_sd {
private:
  uint8_t ea_step;

public:
  Uebergang_Epilleptischer_Anfall(bool aktiv, uint16_t gewichtung, uint16_t steps, uint16_t delay);

  bool doit(struct sKonfiguration *alt, struct sKonfiguration *neu) override;
};

#endif