#include <Arduino.h>

#include "uebergang.h"
#include "uebergang_slot_machine.h"
#include "uebergaenge.h"
#include "einstellungen.h"
#include "konfiguration.h"
#include "led_matrix.h"

#include "uebergang_morph.h"



Uebergang_Slot_Machine::Uebergang_Slot_Machine(bool aktiv, uint16_t gewichtung, uint16_t delay, float speed_min, float speed_max, float damp_min, float damp_max) : Uebergang_d(aktiv, gewichtung, delay) {
  Uebergang_Slot_Machine::speed_min = default_speed_min = speed_min;
  Uebergang_Slot_Machine::speed_max = default_speed_max = speed_max;
  Uebergang_Slot_Machine::damp_min = default_damp_min = damp_min;
  Uebergang_Slot_Machine::damp_max = default_damp_max = damp_max;
  parameter.push_back(P_SPEED_MIN);
  parameter.push_back(P_SPEED_MAX);
  parameter.push_back(P_DAMP_MIN);
  parameter.push_back(P_DAMP_MAX);
  name = (char *)"Slot Machine";
  beschreibung = (char *)"Willkommen in Las Vegas, nur ohne Münzschlitz!";
  tag = (char *)"sm";
  slot_woWirSind = 0;
}

Uebergang_Slot_Machine::~Uebergang_Slot_Machine() {
  if (slot_letzteKonfiguration) {
    free(slot_letzteKonfiguration);
    slot_letzteKonfiguration = nullptr;
  }
}

void Uebergang_Slot_Machine::prefs_laden(Preferences& p) {
  Uebergang_d::prefs_laden(p);
  speed_min = p.getFloat(PREF_SPEED_MIN, speed_min);
  if (speed_min < 0.1 || speed_min > 20) speed_min = default_speed_min;
  speed_max = p.getFloat(PREF_SPEED_MAX, speed_max);
  if (speed_max < 0.1 || speed_max > 20) speed_max = default_speed_max;
  if (speed_max < speed_min) speed_max = speed_min;
  damp_min = p.getFloat(PREF_DAMP_MIN, damp_min);
  if (damp_min < 0.005 || damp_min > 0.5) damp_min = default_damp_min;
  damp_max = p.getFloat(PREF_DAMP_MAX, damp_max);
  if (damp_max < 0.005 || damp_max > 0.5) damp_max = default_damp_max;
  if (damp_max < damp_min) damp_max = damp_min;
}

void Uebergang_Slot_Machine::prefs_schreiben(Preferences& p) {
  Uebergang_d::prefs_schreiben(p);
  if (p.getFloat(PREF_SPEED_MIN) != speed_min) p.putFloat(PREF_SPEED_MIN, speed_min);
  if (p.getFloat(PREF_SPEED_MAX) != speed_max) p.putFloat(PREF_SPEED_MAX, speed_max);
  if (p.getFloat(PREF_DAMP_MIN) != damp_min) p.putFloat(PREF_DAMP_MIN, damp_min);
  if (p.getFloat(PREF_DAMP_MAX) != damp_max) p.putFloat(PREF_DAMP_MAX, damp_max);
}

void Uebergang_Slot_Machine::prefs_ausgeben(String& s) {
  Uebergang_d::prefs_ausgeben(s);
  PREF_AUSGEBEN(s, PREF_SPEED_MIN, speed_min);
  PREF_AUSGEBEN(s, PREF_SPEED_MAX, speed_max);
  PREF_AUSGEBEN(s, PREF_DAMP_MIN, damp_min);
  PREF_AUSGEBEN(s, PREF_DAMP_MAX, damp_max);
}

void Uebergang_Slot_Machine::prefs_defaults() {
  speed_min = default_speed_min;
  speed_max = default_speed_max;
  damp_min = default_damp_min;
  damp_max = default_damp_max;
  Uebergang_d::prefs_defaults();
}

bool Uebergang_Slot_Machine::uebergang_morph(struct sKonfiguration *alt, struct sKonfiguration *neu, uint16_t morph_steps, uint16_t morph_delay, uint16_t morph_delay_ende) {
  struct sKonfiguration x;
  x.r = morph_position(alt->r, neu->r, morph_step, morph_steps);
  x.p = morph_position(alt->p, neu->p, morph_step, morph_steps);
  x.t = morph_position(alt->t, neu->t, morph_step, morph_steps);
  x.u = morph_position(alt->u, neu->u, morph_step, morph_steps);
  x.schriftfarbe = morph_color(alt->schriftfarbe, neu->schriftfarbe, morph_step, morph_steps);
  x.hintergrundfarbe = morph_color(alt->hintergrundfarbe, neu->hintergrundfarbe, morph_step, morph_steps);
  x.wrapY = true;

  morph_step++;
  uebergang_queueKonfiguration(&x, ((morph_step == morph_steps + 1) && (morph_steps>3)) ? morph_delay_ende : morph_delay);
  
  if (morph_step >= morph_steps + 1) {
    morph_step = 0;
    return true;
  }
  return false;
}

bool Uebergang_Slot_Machine::doit(struct sKonfiguration *alt, struct sKonfiguration *neu) {
  if (slot_woWirSind == 0) { // wir fangen gerade an, also hauen wir zwei Bitmaps mit Shift nach unten raus.
    // Wir fangen einfach mit der neuen Farbe, aber der alten Position an.
    // und bauen daraus eine neue Konfiguration, die noch zwei Positionen tiefer bei allem ist.
    struct sKonfiguration *k = clone_konfiguration(alt);
    k->r.y += 2;
    k->p.y += 2;
    k->t.y += 2;
    k->u.y += 2;
    k->wrapY = true;
    // da bewegen wir uns linear hin.
    while (!uebergang_morph(alt, k, 2, 200, 200));
    free(k);
    slot_gesamt = 0;
    slot_woWirSind++;
    return false; // noch nicht fertig
  }
  if (slot_woWirSind == 1) {
    // Wir berechnen das alles rückwärts. Wir fangen mit der Zielposition an.
    slot_positions[0] = neu->r.y;
    slot_positions[1] = neu->p.y;
    slot_positions[2] = neu->t.y;
    slot_positions[3] = neu->u.y;
    // und als initiale Geschwindigkeit nehmen wir einfach den Wert 1.
    slot_speeds[0] = speed_min;
    slot_speeds[1] = speed_min;
    slot_speeds[2] = speed_min;
    slot_speeds[3] = speed_min;
    // Dann würfeln wir noch leicht unterschiedliche Dämpfungen. Wir merken uns aber den Kehrwert, weil Multiplizieren billiger ist als dividieren.
    for (int i = 0; i < 4; i++) {
      slot_dampening[i] = 1.0f / (1.0f - (damp_min + (float)rand()/(float)((RAND_MAX/(damp_max - damp_min)))));
    }
    // Und jetzt berechnen wir Konfigurationen, bis wir die Maximalgeschwindigkeit auf einem Slot erreicht haben. Jede Konfiguration kennt dabei die nächste (linked list).
    struct sKonfiguration *ka = clone_konfiguration(neu);
    while (slot_speeds[0] < speed_max && slot_speeds[1] < speed_max && slot_speeds[2] < speed_max && slot_speeds[3] < speed_max) {
      slot_gesamt++;
      for (int i = 0; i < 4; i++) {
        slot_positions[i] += slot_speeds[i];
        // alles normalisieren
        while (slot_positions[i] < 0) slot_positions[i] += LED_COUNT_Y;
        while (slot_positions[i] >= LED_COUNT_Y) slot_positions[i] -= LED_COUNT_Y;
        // und die Geschwindigkeit erhöhen
        slot_speeds[i] *= slot_dampening[i];
      }
      struct sKonfiguration *k = clone_konfiguration(neu);
      k->r.y = lroundf(slot_positions[0]);
      k->p.y = lroundf(slot_positions[1]);
      k->t.y = lroundf(slot_positions[2]);
      k->u.y = lroundf(slot_positions[3]);
      k->wrapY = true;
      k->naechste = ka;
      ka = k;
    }
    slot_letzteKonfiguration = ka;
    slot_count = 0;
    slot_woWirSind++;
    return false; // noch nicht fertig, aber wir wollen die CPU nicht länger belegen
  }
  if (slot_woWirSind == 2) {
    // Und jetzt wickeln wir das Ganze rückwärts ab und fixen noch die Farben
    slot_letzteKonfiguration->schriftfarbe = morph_color(alt->schriftfarbe, neu->schriftfarbe, slot_count, slot_gesamt);
    slot_letzteKonfiguration->hintergrundfarbe = morph_color(alt->hintergrundfarbe, neu->hintergrundfarbe, slot_count, slot_gesamt);
    slot_count++;

    uebergang_queueKonfiguration(slot_letzteKonfiguration, slot_letzteKonfiguration->naechste == NULL ? konfiguration_pause : delay);

    struct sKonfiguration *freeMe = slot_letzteKonfiguration; 
    slot_letzteKonfiguration = slot_letzteKonfiguration->naechste;
    free(freeMe);

    if (!slot_letzteKonfiguration) {
      slot_woWirSind = 0;
      return true;
    }
    return false;
  }
  return true;
}