#include <Arduino.h>
#include "gimp_artwork.h"
#include "led_matrix.h"
#include "defaults.h"


// alpha_faktor: 0 - 256. 256 heißt: Daten so wie im Bild angegeben. Weniger heißt: durchscheinender.
void gimp_rendern(struct sCRGBA *bitmap, const struct sGIMP *gimp, int16_t ofsx, int16_t ofsy, uint8_t refpunkt, uint16_t alpha_faktor) {
  int16_t origin_x = 0;
  int16_t origin_y = 0;
  switch (refpunkt) {
  case REFPUNKT_OBEN_LINKS:
    origin_x = ofsx;
    origin_y = ofsy;
    break;
  case REFPUNKT_MITTE:
    origin_x = ofsx - gimp->width / 2;
    origin_y = ofsy - gimp->height / 2;
    break;
  }
  const char *c = gimp->data;
  for (int16_t y = origin_y; y < origin_y + gimp->height; y++) {
    for (int16_t x = origin_x; x < origin_x + gimp->width; x++) {
      struct sCRGBA pixel;
      pixel.r = (uint8_t) *(c++);
      pixel.g = (uint8_t) *(c++);
      pixel.b = (uint8_t) *(c++);
      pixel.alpha = ((uint16_t) *(c++)) * alpha_faktor / 256;
      if (x >= 0 && x < LED_COUNT_X && y >= 0 && y < LED_COUNT_Y) {
        bitmap[y * LED_COUNT_X + x] = pixel;
      }
    }
  }
}

