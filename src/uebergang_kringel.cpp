#include <Arduino.h>
#include <Preferences.h>
#include <math.h>

#include "uebergang.h"
#include "uebergang_kringel.h"
#include "led_matrix.h"
#include "konfiguration.h"
#include "einstellungen.h"
#include "uebergaenge.h"



#define MITTELPUNKT_X ((float)(LED_COUNT_X-1) / 2.0)
#define MITTELPUNKT_Y ((float)(LED_COUNT_Y-1) / 2.0)

static struct sKringelPol uebergang_kringel_PolarAusKartesisch(struct sPosition k) {
  struct sKringelPol p;
  float mpkx = (k.x + 3.5) - MITTELPUNKT_X;
  float mpky = -((k.y + 3.5) - MITTELPUNKT_Y);
  p.radius = sqrt(mpkx * mpkx + mpky * mpky);
  p.winkel = atan2f(mpky, mpkx);
  return p;
}

static struct sPosition uebergang_kringel_kartesischAusPolar(struct sKringelPol p) {
  struct sPosition k;
  k.x = roundf(MITTELPUNKT_X + (cos(p.winkel) * p.radius) - 3.5);
  k.y = roundf(MITTELPUNKT_Y + (sin(p.winkel) * p.radius) - 3.5);
  return k;
}

static struct sKringelPol uebergang_kringel_polarinterpolation(struct sKringelSZ sz, float step, int8_t richtung) {
  float istep = 1.0f - step;
  struct sKringelPol p;
  p.winkel = sz.start.winkel * istep + sz.ziel.winkel * step;
  p.radius = sz.start.radius * istep + sz.ziel.radius * step;
  return p;
}

static struct sKringelSZ uebergang_kringel_berechneStartZiel(struct sPosition alt, struct sPosition neu, int8_t richtung) {
  struct sKringelSZ s;
  s.start = uebergang_kringel_PolarAusKartesisch(alt);
  s.ziel = uebergang_kringel_PolarAusKartesisch(neu);
  if (richtung < 0) {
    s.start.winkel += 2 * PI;
  }
  else if (richtung > 0) {
    s.start.winkel -= 2 * PI;
  }
  return s;
}

Uebergang_Kringel::Uebergang_Kringel(bool aktiv, uint16_t gewichtung, uint16_t steps, uint16_t delay, int8_t richtung) : Uebergang_sd(aktiv, gewichtung, steps, delay) {
  Uebergang_Kringel::richtung = richtung;
  name = (char *)"Kringel";
  beschreibung = (char *)"Buchstaben rotieren mit unterschiedlicher Geschwindigkeit und wechseln dabei gegebenenfalls nach und nach die Umlaufbahn. Rechtsrum oder linksrum.";
  tag = (char *)"kringel";
  kringel_step = 0;
}

Uebergang_Kringel_linksrum::Uebergang_Kringel_linksrum(bool aktiv, uint16_t gewichtung, uint16_t steps, uint16_t delay) : Uebergang_Kringel(aktiv, gewichtung, steps, delay, -1) {
  name = (char *)"Kringel linksrum";
  tag = (char *)"kringel_l";
}

Uebergang_Kringel_rechtsrum::Uebergang_Kringel_rechtsrum(bool aktiv, uint16_t gewichtung, uint16_t steps, uint16_t delay) : Uebergang_Kringel(aktiv, gewichtung, steps, delay, +1) {
  name = (char *)"Kringel rechtsrum";
  tag = (char *)"kringel_r";
}

bool Uebergang_Kringel::doit(struct sKonfiguration *alt, struct sKonfiguration *neu) {
  if (kringel_step == 0) {
    kpr = uebergang_kringel_berechneStartZiel(alt->r, neu->r, richtung);
    kpp = uebergang_kringel_berechneStartZiel(alt->p, neu->p, richtung);
    kpt = uebergang_kringel_berechneStartZiel(alt->t, neu->t, richtung);
    kpu = uebergang_kringel_berechneStartZiel(alt->u, neu->u, richtung);
  }
  float step = (float)kringel_step / steps;
  struct sKonfiguration x;
  x.r = uebergang_kringel_kartesischAusPolar(uebergang_kringel_polarinterpolation(kpr, step, richtung));
  x.p = uebergang_kringel_kartesischAusPolar(uebergang_kringel_polarinterpolation(kpp, step, richtung));
  x.t = uebergang_kringel_kartesischAusPolar(uebergang_kringel_polarinterpolation(kpt, step, richtung));
  x.u = uebergang_kringel_kartesischAusPolar(uebergang_kringel_polarinterpolation(kpu, step, richtung));
  x.schriftfarbe = morph_color(alt->schriftfarbe, neu->schriftfarbe, kringel_step, steps);
  x.hintergrundfarbe = morph_color(alt->hintergrundfarbe, neu->hintergrundfarbe, kringel_step, steps);

  kringel_step++;
  uebergang_queueKonfiguration(&x, (kringel_step == steps + 1) ? konfiguration_pause : delay);
  
  if (kringel_step >= steps + 1) {
    kringel_step = 0;
    return true;
  }
  return false;
}
