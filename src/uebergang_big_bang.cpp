#include <Arduino.h>

#include "uebergang_big_bang.h"
#include "einstellungen.h"
#include "led_matrix.h"
#include "uebergaenge.h"
#include "uebergang_morph.h"
#include "konfiguration.h"



Uebergang_Big_Bang::Uebergang_Big_Bang(bool aktiv, uint16_t gewichtung, uint16_t steps, uint16_t delay) : Uebergang_sd(aktiv, gewichtung, steps, delay) {
  name = (char *)"Big Bang";
  beschreibung = (char *)"Buchstaben kollabieren im Gravitationszentrum und explodieren von dort in eine neue Position, wie Materie in einem oszillierenden Universum.";
  tag = (char *)"big_bang";
  bigbang_phase = 0;
  morph_step = 0;
}


bool Uebergang_Big_Bang::uebergang_morph(struct sKonfiguration *alt, struct sKonfiguration *neu, uint16_t steps, uint16_t delay, uint16_t delay_end) {
  struct sKonfiguration x;
  x.r = morph_position(alt->r, neu->r, morph_step, steps);
  x.p = morph_position(alt->p, neu->p, morph_step, steps);
  x.t = morph_position(alt->t, neu->t, morph_step, steps);
  x.u = morph_position(alt->u, neu->u, morph_step, steps);
  x.schriftfarbe = morph_color(alt->schriftfarbe, neu->schriftfarbe, morph_step, steps);
  x.hintergrundfarbe = morph_color(alt->hintergrundfarbe, neu->hintergrundfarbe, morph_step, steps);

  morph_step++;
  uebergang_queueKonfiguration(&x, ((morph_step >= steps + 1) && (steps>3)) ? delay_end : delay);
  
  if (morph_step >= steps + 1) {
    morph_step = 0;
    return true;
  }
  return false;
}



bool Uebergang_Big_Bang::doit(struct sKonfiguration *alt, struct sKonfiguration *neu) {
  bool fertig;
  switch (bigbang_phase) {
  case 0:
    bigbang_mitte.r.x = (LED_COUNT_X - 8) / 2; bigbang_mitte.r.y = (LED_COUNT_Y - 8) / 2;
    bigbang_mitte.p.x = (LED_COUNT_X - 8) / 2; bigbang_mitte.p.y = (LED_COUNT_Y - 8) / 2;
    bigbang_mitte.t.x = (LED_COUNT_X - 8) / 2; bigbang_mitte.t.y = (LED_COUNT_Y - 8) / 2;
    bigbang_mitte.u.x = (LED_COUNT_X - 8) / 2; bigbang_mitte.u.y = (LED_COUNT_Y - 8) / 2;
    bigbang_mitte.schriftfarbe = morph_color(alt->schriftfarbe, neu->schriftfarbe, 1, 2);
    bigbang_mitte.hintergrundfarbe = morph_color(alt->hintergrundfarbe, neu->hintergrundfarbe, 1, 2);
    bigbang_phase = 1;
    return false;
  case 1:
    fertig = Uebergang_Big_Bang::uebergang_morph(alt, &bigbang_mitte, steps/2, delay, delay);
    if (fertig) bigbang_phase = 2;
    return false;
  case 2:
    fertig = Uebergang_Big_Bang::uebergang_morph(&bigbang_mitte, neu, steps/2, delay, konfiguration_pause);
    if (fertig) bigbang_phase = 3;
    return false;
  default:
    bigbang_phase = 0;
    return true;
  }

}
