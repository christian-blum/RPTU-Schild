#include <Arduino.h>
#include <Preferences.h>

#include "uebergang.h"
#include "uebergang_morph.h"
#include "einstellungen.h"
#include "led_matrix.h"
#include "konfiguration.h"
#include "uebergaenge.h"



Uebergang_Morph::Uebergang_Morph(bool aktiv, uint16_t gewichtung, uint16_t steps, uint16_t delay) : Uebergang_sd(aktiv, gewichtung, steps, delay) {
  name = (char *)"Morph";
  beschreibung = (char *)"Buchstaben bewegen sich in ihre neue Position, dabei ändern sich Farben kontinuierlich.";
  tag = (char *)"morph";
  morph_step = 0;
}

bool Uebergang_Morph::doit(struct sKonfiguration *alt, struct sKonfiguration *neu) {
  struct sKonfiguration x;
  x.r = morph_position(alt->r, neu->r, morph_step, steps);
  x.p = morph_position(alt->p, neu->p, morph_step, steps);
  x.t = morph_position(alt->t, neu->t, morph_step, steps);
  x.u = morph_position(alt->u, neu->u, morph_step, steps);
  x.schriftfarbe = morph_color(alt->schriftfarbe, neu->schriftfarbe, morph_step, steps);
  x.hintergrundfarbe = morph_color(alt->hintergrundfarbe, neu->hintergrundfarbe, morph_step, steps);

  morph_step++;
  uebergang_queueKonfiguration(&x, ((morph_step >= steps + 1) && (steps>3)) ? konfiguration_pause : delay);
  
  if (morph_step >= steps + 1) {
    morph_step = 0;
    return true;
  }
  return false;
}
