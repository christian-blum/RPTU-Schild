#ifndef __EFFEKT_GIMP
#define __EFFEKT_GIMP

#include "effekt.h"
#include "gimp_artwork.h"

class Effekt_GIMP : public Effekt {

  const struct sGIMP *artwork;

  public:
    Effekt_GIMP(bool loeschbar, bool aktiv, uint16_t gewichtung, const struct sGIMP *artwork, uint16_t dauer);

    uint16_t default_dauer;
    uint16_t dauer;

    bool doit() override;

};

#endif