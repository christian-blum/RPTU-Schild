#ifndef __UEBERGANG_KRINGEL_H
#define __UEBERGANG_KRINGEL_H

#include <Arduino.h>
#include <Preferences.h>
#include "uebergang.h"


struct sKringelPol {
  float winkel;
  float radius;
};

struct sKringelSZ {
  struct sKringelPol start;
  struct sKringelPol ziel;
};



class Uebergang_Kringel : public Uebergang_sd {
private:
  uint16_t kringel_step;
  struct sKringelSZ kpr;
  struct sKringelSZ kpp;
  struct sKringelSZ kpt;
  struct sKringelSZ kpu;


public:
  Uebergang_Kringel(bool aktiv, uint16_t gewichtung, uint16_t steps, uint16_t delay, int8_t richtung); // +1 oder -1
  int8_t richtung;

  bool doit(struct sKonfiguration *alt, struct sKonfiguration *neu) override;
};

class Uebergang_Kringel_linksrum : public Uebergang_Kringel {
public:
  Uebergang_Kringel_linksrum(bool aktiv, uint16_t gewichtung, uint16_t steps, uint16_t delay);
};

class Uebergang_Kringel_rechtsrum : public Uebergang_Kringel {
public:
  Uebergang_Kringel_rechtsrum(bool aktiv, uint16_t gewichtung, uint16_t steps, uint16_t delay);
};

#endif