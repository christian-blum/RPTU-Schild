#ifndef __UEBERGANG_JUMP_H
#define __UEBERGANG_JUMP_H

#include "uebergang.h"

bool uebergang_jump(struct sKonfiguration *alteKonfiguration, struct sKonfiguration *neueKonfiguration);

class Uebergang_Jump : public Uebergang {
public:

  void prefs_laden(Preferences& p) override;
  void prefs_schreiben(Preferences& p) override;
  void prefs_ausgeben(String& s) override;
  bool doit(struct sKonfiguration *alt, struct sKonfiguration *neu) override;
};

#endif
