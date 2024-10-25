#include "effekt_gimp.h"

Effekt_GIMP::Effekt_GIMP(bool aktiv, uint16_t gewichtung, struct sGIMP *artwork, uint16_t dauer) : Effekt(aktiv, gewichtung) {
  Effekt_GIMP::artwork = artwork;
  Effekt_GIMP::dauer = dauer;
}

