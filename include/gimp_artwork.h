#ifndef __GIMP_ARTWORK_H
#define __GIMP_ARTWORK_H

#include <Arduino.h>

struct sGIMP {
  const uint16_t width;
  const uint16_t height;
  const uint8_t bytes_per_pixel;
  const char *comment;
  const char *data;
};

enum gimp_artwork_eRefpunkt {
  REFPUNKT_OBEN_LINKS,
  REFPUNKT_MITTE
};

void gimp_rendern(struct sCRGBA *bitmap, const struct sGIMP *gimp, int16_t ofsx, int16_t ofsy, uint8_t refpunkt, uint16_t alpha_faktor);

#endif