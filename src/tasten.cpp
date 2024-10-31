#include <Arduino.h>

#include "tasten.h"
#include "einstellungen.h"
#include "pins.h"
#include "osd.h"
#include "cb_scheduler.h"
#include "led_matrix.h"
#include "defaults.h"

/**************************************************************
 *                                                            *
 * TASTEN                                                     *
 *                                                            *
 **************************************************************/

#define ENTPRELLUNG_MAX 10
#define ENTPRELLUNG_SCHWELLE 7


Taste t_einaus(EINAUS_PIN);
Taste t_dunkler(DUNKLER_PIN);
Taste t_heller(HELLER_PIN);
Taste t_effekt(EFFEKTE_EINAUS_PIN);
#ifdef HAVE_BLUETOOTH
Taste t_bluetooth(BT_EINAUSPAIRINGCLEAR_PIN);
#endif

void tasten_setup() {
  t_einaus.begin();
  t_heller.begin();
  t_dunkler.begin();
  t_effekt.begin();
#ifdef HAVE_BLUETOOTH
  t_bluetooth.begin();
#endif
}



void t_heller_repeat() {
  if (t_heller.aktiv()) {
    semaphore_osd_helligkeit = true;
    if (helligkeit < HELLIGKEIT_MAX) {
      helligkeit++;
      scheduler.callMeInMilliseconds(t_heller_repeat, TASTENWIEDERHOLZEIT);
      semaphore_ledMatrix_update = true;
      preferences_speichern = true;
    }
  }
}

void t_dunkler_repeat() {
  if (t_dunkler.aktiv()) {
    semaphore_osd_helligkeit = true;
    if (helligkeit > HELLIGKEIT_MIN) {
      helligkeit--;
      scheduler.callMeInMilliseconds(t_dunkler_repeat, TASTENWIEDERHOLZEIT);
      semaphore_ledMatrix_update = true;
      preferences_speichern = true;
    }
  }
}


void tasten_loop() {
  t_einaus.loop();
  t_heller.loop();
  t_dunkler.loop();
  t_effekt.loop();
#ifdef HAVE_BLUETOOTH
  t_bluetooth.loop();
#endif

  if (t_einaus.zustandswechsel()) {
    if (t_einaus.aktiv()) {
      einaus = !einaus;
      preferences_speichern = true;
    }
  }
  if (t_heller.zustandswechsel()) {
    t_heller_repeat();
  }
  if (t_dunkler.zustandswechsel()) {
    t_dunkler_repeat();
  }
  if (t_effekt.zustandswechsel()) {
    if (t_effekt.aktiv()) {
      semaphore_osd_effekte = true;
      if (osd_effekte_sichtbar) {
        effekte_einaus = !effekte_einaus;
      }
      preferences_speichern = true;
    }
  }
}


Taste::Taste(int pin) {
  Taste::pin = pin;
}

void Taste::begin() {
  pinMode(pin, INPUT_PULLDOWN);
}

void Taste::loop() {
  bool x = digitalRead(pin);
  if (x && count < ENTPRELLUNG_MAX) count++;
  else if (!x && count > -ENTPRELLUNG_MAX) count--;
  bool z = aktiv();
  if (z != zustand) {
    aenderung = true;
    zustand = z;
  }
}

bool Taste::aktiv() {
  return count >= ENTPRELLUNG_SCHWELLE;
}

bool Taste::zustandswechsel() {
  bool zw = aenderung;
  aenderung = false;
  return zw;
}