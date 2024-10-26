#include "effekt_gimp.h"
#include "led_matrix.h"

Effekt_GIMP::Effekt_GIMP(bool loeschbar, bool aktiv, uint16_t gewichtung, const struct sGIMP *artwork, uint16_t dauer) : Effekt(loeschbar, aktiv, gewichtung) {
// FIXME name. desc als Argumente übergeben und speichern
  Effekt_GIMP::artwork = artwork;
  Effekt_GIMP::dauer = Effekt_GIMP::default_dauer = dauer;
}

bool Effekt_GIMP::doit() {
  struct sBitmap *b = (struct sBitmap *) malloc(sizeof(struct sBitmap));
  memset(b, 0, sizeof(struct sBitmap));
  struct sCRGBA *bitmap = (struct sCRGBA *) calloc(LED_COUNT, sizeof(struct sCRGBA));
  memset(bitmap, 0, sizeof(struct sCRGBA) * LED_COUNT);
  gimp_rendern(bitmap, artwork, LED_COUNT_X / 2, LED_COUNT_Y / 2, REFPUNKT_MITTE, 192);
  b->bitmap = bitmap;
  uint32_t milliseconds = 5000;
  b->milliseconds = milliseconds;
  effekt_queue_bitmap(b);
  return true;
}