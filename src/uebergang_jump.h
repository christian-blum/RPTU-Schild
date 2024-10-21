#ifndef __UEBERGANG_JUMP_H
#define __UEBERGANG_JUMP_H

#include "uebergang.h"

class Uebergang_Jump : public Uebergang {
public:
  Uebergang_Jump(bool aktiv, uint16_t gewichtung);

  bool doit(struct sKonfiguration *alt, struct sKonfiguration *neu) override;
};

#endif
