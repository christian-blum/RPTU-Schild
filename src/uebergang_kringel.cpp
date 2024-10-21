#include <Arduino.h>
#include <Preferences.h>
#include <math.h>

#include "uebergang.h"
#include "uebergang_kringel.h"
#include "led_matrix.h"
#include "konfiguration.h"
#include "einstellungen.h"
#include "uebergaenge.h"


uint16_t uebergang_kringel_steps;
uint16_t uebergang_kringel_delay;

#define PREF_KRINGEL_STEPS "kringel_steps"
#define PREF_KRINGEL_DELAY "kringel_delay"


void uebergang_kringel_prefs_laden(Preferences& p) {
  uebergang_kringel_steps = p.getUShort(PREF_KRINGEL_STEPS, KRINGEL_STEPS);
  uebergang_kringel_delay = p.getUShort(PREF_KRINGEL_DELAY, KRINGEL_DELAY);
}

void uebergang_kringel_prefs_ausgeben(String& s) {
  PREF_APPEND(s, PREF_KRINGEL_STEPS, uebergang_kringel_steps);
  PREF_APPEND(s, PREF_KRINGEL_DELAY, uebergang_kringel_delay);
}

void uebergang_kringel_prefs_schreiben(Preferences& p) {
  if (p.getUShort(PREF_KRINGEL_STEPS) != uebergang_kringel_steps) p.putUShort(PREF_KRINGEL_STEPS, uebergang_kringel_steps);
  if (p.getUShort(PREF_KRINGEL_DELAY) != uebergang_kringel_delay) p.putUShort(PREF_KRINGEL_DELAY, uebergang_kringel_delay);
}

struct sKringelPol {
  float winkel;
  float radius;
};

struct sKringelSZ {
  struct sKringelPol start;
  struct sKringelPol ziel;
};

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


static uint16_t kringel_step;
static struct sKringelSZ kpr;
static struct sKringelSZ kpp;
static struct sKringelSZ kpt;
static struct sKringelSZ kpu;

/*
static void printIt(struct sPosition k, sKringelPol p) {
    struct sPosition a = uebergang_kringel_kartesischAusPolar(p);
}
*/

bool uebergang_kringel(struct sKonfiguration *alt, struct sKonfiguration *neu, int8_t winkelvorzeichen) {
  if (kringel_step == 0) {
    kpr = uebergang_kringel_berechneStartZiel(alt->r, neu->r, winkelvorzeichen);
    kpp = uebergang_kringel_berechneStartZiel(alt->p, neu->p, winkelvorzeichen);
    kpt = uebergang_kringel_berechneStartZiel(alt->t, neu->t, winkelvorzeichen);
    kpu = uebergang_kringel_berechneStartZiel(alt->u, neu->u, winkelvorzeichen);
  }
  float step = (float)kringel_step / uebergang_kringel_steps;
  struct sKonfiguration x;
  x.r = uebergang_kringel_kartesischAusPolar(uebergang_kringel_polarinterpolation(kpr, step, winkelvorzeichen));
  x.p = uebergang_kringel_kartesischAusPolar(uebergang_kringel_polarinterpolation(kpp, step, winkelvorzeichen));
  x.t = uebergang_kringel_kartesischAusPolar(uebergang_kringel_polarinterpolation(kpt, step, winkelvorzeichen));
  x.u = uebergang_kringel_kartesischAusPolar(uebergang_kringel_polarinterpolation(kpu, step, winkelvorzeichen));
  x.schriftfarbe = morph_color(alt->schriftfarbe, neu->schriftfarbe, kringel_step, uebergang_kringel_steps);
  x.hintergrundfarbe = morph_color(alt->hintergrundfarbe, neu->hintergrundfarbe, kringel_step, uebergang_kringel_steps);

  kringel_step++;
  uebergang_queueKonfiguration(&x, (kringel_step == uebergang_kringel_steps + 1) ? konfiguration_pause : uebergang_kringel_delay);
  
  if (kringel_step >= uebergang_kringel_steps + 1) {
    kringel_step = 0;
    return true;
  }
  return false;
}


bool uebergang_kringel_rechtsrum(struct sKonfiguration *alt, struct sKonfiguration *neu) {
  return uebergang_kringel(alt, neu, +1);
}

bool uebergang_kringel_linksrum(struct sKonfiguration *alt, struct sKonfiguration *neu) {
  return uebergang_kringel(alt, neu, -1);
}

Uebergang_Kringel::Uebergang_Kringel(bool aktiv, uint16_t gewichtung, uint16_t steps, uint16_t delay, int8_t richtung) : Uebergang(aktiv, gewichtung) {
  Uebergang_Kringel::steps = default_steps = steps;
  Uebergang_Kringel::delay = default_delay = delay;
  Uebergang_Kringel::richtung = richtung;
}

Uebergang_Kringel_linksrum::Uebergang_Kringel_linksrum(bool aktiv, uint16_t gewichtung, uint16_t steps, uint16_t delay) : Uebergang_Kringel(aktiv, gewichtung, steps, delay, -1) {
}

Uebergang_Kringel_rechtsrum::Uebergang_Kringel_rechtsrum(bool aktiv, uint16_t gewichtung, uint16_t steps, uint16_t delay) : Uebergang_Kringel(aktiv, gewichtung, steps, delay, +1) {
}

void Uebergang_Kringel::prefs_laden(Preferences& p) {
  uebergang_kringel_prefs_laden(p);
}

void Uebergang_Kringel::prefs_schreiben(Preferences& p) {
  uebergang_kringel_prefs_schreiben(p);
}

void Uebergang_Kringel::prefs_ausgeben(String& s) {
  uebergang_kringel_prefs_ausgeben(s);
}