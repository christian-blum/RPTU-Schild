#include <Arduino.h>

#include "uebergang_slot_machine.h"
#include "uebergaenge.h"
#include "einstellungen.h"
#include "konfiguration.h"
#include "led_matrix.h"

#include "uebergang_morph.h"

#define PREF_SLOT_SPEED_MAX "slot_speed_max"
#define PREF_SLOT_SPEED_MIN "slot_speed_min"
#define PREF_SLOT_DAMP_MAX "slot_damp_max"
#define PREF_SLOT_DAMP_MIN "slot_damp_min"
#define PREF_SLOT_DELAY "slot_delay"


float uebergang_slot_speed_max;
float uebergang_slot_speed_min;
float uebergang_slot_damp_max;
float uebergang_slot_damp_min;
uint16_t uebergang_slot_delay;


static uint8_t slot_woWirSind;
static float slot_positions[4];
static float slot_speeds[4];
static float slot_dampening[4];
static struct sKonfiguration * slot_letzteKonfiguration;
static uint32_t slot_gesamt;
static uint32_t slot_count;



void uebergang_slot_machine_prefs_laden(Preferences p) {
  uebergang_slot_speed_max = p.getFloat(PREF_SLOT_SPEED_MAX, SLOT_SPEED_MAX);
  uebergang_slot_speed_min = p.getFloat(PREF_SLOT_SPEED_MIN, SLOT_SPEED_MIN);
  uebergang_slot_damp_max = p.getFloat(PREF_SLOT_DAMP_MAX, SLOT_DAMP_MAX);
  uebergang_slot_damp_min = p.getFloat(PREF_SLOT_DAMP_MIN, SLOT_DAMP_MIN);
  uebergang_slot_delay = p.getUShort(PREF_SLOT_DELAY, SLOT_DELAY);
}

String uebergang_slot_machine_prefs_ausgeben() {
  return PREF_TO_STRING(PREF_SLOT_SPEED_MAX, uebergang_slot_speed_max) +
    PREF_TO_STRING(PREF_SLOT_SPEED_MIN, uebergang_slot_speed_min) +
    PREF_TO_STRING(PREF_SLOT_DAMP_MAX, uebergang_slot_damp_max) +
    PREF_TO_STRING(PREF_SLOT_DAMP_MIN, uebergang_slot_damp_min) +
    PREF_TO_STRING(PREF_SLOT_DELAY, uebergang_slot_delay);
}

void uebergang_slot_machine_prefs_schreiben(Preferences p) {
  if (p.getFloat(PREF_SLOT_SPEED_MAX) != uebergang_slot_speed_max) p.putFloat(PREF_SLOT_SPEED_MAX, uebergang_slot_speed_max);
  if (p.getFloat(PREF_SLOT_SPEED_MIN) != uebergang_slot_speed_min) p.putFloat(PREF_SLOT_SPEED_MIN, uebergang_slot_speed_min);
  if (p.getFloat(PREF_SLOT_DAMP_MAX) != uebergang_slot_damp_max) p.putFloat(PREF_SLOT_DAMP_MAX, uebergang_slot_damp_max);
  if (p.getFloat(PREF_SLOT_DAMP_MIN) != uebergang_slot_damp_min) p.putFloat(PREF_SLOT_DAMP_MIN, uebergang_slot_damp_min);
  if (p.getUShort(PREF_SLOT_DELAY) != uebergang_slot_delay) p.putUShort(PREF_SLOT_DELAY, uebergang_slot_delay);
}







bool uebergang_slot_machine(struct sKonfiguration *alt, struct sKonfiguration *neu) {
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
    slot_speeds[0] = uebergang_slot_speed_min;
    slot_speeds[1] = uebergang_slot_speed_min;
    slot_speeds[2] = uebergang_slot_speed_min;
    slot_speeds[3] = uebergang_slot_speed_min;
    // Dann würfeln wir noch leicht unterschiedliche Dämpfungen. Wir merken uns aber den Kehrwert, weil Multiplizieren billiger ist als dividieren.
    for (int i = 0; i < 4; i++) {
      slot_dampening[i] = 1.0f / (1.0f - (uebergang_slot_damp_min + (float)rand()/(float)((RAND_MAX/(uebergang_slot_damp_max - uebergang_slot_damp_min)))));
    }
    // Und jetzt berechnen wir Konfigurationen, bis wir die Maximalgeschwindigkeit auf einem Slot erreicht haben. Jede Konfiguration kennt dabei die nächste (linked list).
    struct sKonfiguration *ka = clone_konfiguration(neu);
    while (slot_speeds[0] < uebergang_slot_speed_max && slot_speeds[1] < uebergang_slot_speed_max && slot_speeds[2] < uebergang_slot_speed_max && slot_speeds[3] < uebergang_slot_speed_max) {
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

    uebergang_queueKonfiguration(slot_letzteKonfiguration, slot_letzteKonfiguration->naechste == NULL ? konfiguration_pause : uebergang_slot_delay);

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
