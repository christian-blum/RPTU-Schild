#ifndef __EFFEKT_PAUSE
#define __EFFEKT_PAUSE

#include "effekt.h"


class Effekt_Pause : public Effekt {
  private:
    int count;

    void queue_pause(uint32_t millis);

  public:
    Effekt_Pause();

    bool doit() override;
};

#endif