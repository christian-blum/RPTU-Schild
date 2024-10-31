#include <Arduino.h>
#include "led_matrix.h"
#include "effekt_pause.h"
#include "einstellungen.h"
#include "effekte.h"

Effekt_Pause::Effekt_Pause() : Effekt(false, false, 0) {
  count = 0;
}

void Effekt_Pause::queue_pause(uint32_t millis) {
  struct sBitmap *b = (struct sBitmap *) malloc(sizeof(struct sBitmap));
  memset(b, 0, sizeof(struct sBitmap));
  b->bitmap = NULL;
  b->milliseconds = millis;
  effekt_queue_bitmap(b);
}

bool Effekt_Pause::doit() {
  if (count == 0) {
    queue_pause(random(effekt_pause_max - effekt_pause_min) + effekt_pause_min);
  }
  else {
    queue_pause(100);  // das ist ein Hack, der dazu dient, die Pipeline mit kurzlebigem Zeug gefüllt zu halten
  }
  count++;
  if (count > EFFEKTE_PIPELINE_MINDESTLAENGE) {
    count = 0;
    return true;
  }
  return false;
}