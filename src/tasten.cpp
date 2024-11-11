#include <Arduino.h>
#include <KeyDebouncer.h>

#include "tasten.h"
#include "einstellungen.h"
#include "pins.h"
#include "osd.h"
#include "led_matrix.h"
#include "defaults.h"

/**************************************************************
 *                                                            *
 * TASTEN                                                     *
 *                                                            *
 **************************************************************/

KeyDebouncer taste_einaus(EINAUS_PIN, false);
KeyDebouncer taste_dunkler(DUNKLER_PIN, false, TASTENWIEDERHOLZEIT * 1000, 0);
KeyDebouncer taste_heller(HELLER_PIN, false, TASTENWIEDERHOLZEIT * 1000, 0);
KeyDebouncer taste_effekt(EFFEKTE_EINAUS_PIN, false);
#ifdef HAVE_BLUETOOTH
KeyDebouncer taste_bluetooth(BT_EINAUSPAIRINGCLEAR_PIN, false);
#endif


static void taste_einaus_gedrueckt() {
  einaus = !einaus;
  semaphore_ledMatrix_update = true;
  preferences_speichern = true;
}

static void taste_heller_gedrueckt() {
  semaphore_osd_helligkeit = true;
  if (helligkeit < HELLIGKEIT_MAX) {
    helligkeit++;
    semaphore_ledMatrix_update = true;
    preferences_speichern = true;
  }
}

static void taste_dunkler_gedrueckt() {
  semaphore_osd_helligkeit = true;
  if (helligkeit > HELLIGKEIT_MIN) {
    helligkeit--;
    semaphore_ledMatrix_update = true;
    preferences_speichern = true;
  }
}

static void taste_effekt_gedrueckt() {
  semaphore_osd_effekte = true;
  if (osd_effekte_sichtbar) {
    effekte_einaus = !effekte_einaus;
  }
  preferences_speichern = true;
}

#ifdef HAVE_BLUETOOTH

static void taste_bluetooth_gedrueckt() {
  // FIXME das müsste halt noch jemand schreiben :-)
}

#endif

void tasten_setup() {
  taste_einaus.begin();
  taste_einaus.callMeIfPressedOnLoop(taste_einaus_gedrueckt);
  taste_heller.begin();
  taste_heller.callMeIfPressedOnLoop(taste_heller_gedrueckt);
  taste_dunkler.begin();
  taste_dunkler.callMeIfPressedOnLoop(taste_dunkler_gedrueckt);
  taste_effekt.begin();
  taste_effekt.callMeIfPressedOnLoop(taste_effekt_gedrueckt);
#ifdef HAVE_BLUETOOTH
  taste_bluetooth.begin();
  taste_bluetooth.callMeIfPressedOnLoop(taste_bluetooth_gedrueckt);
#endif
}

void tasten_loop() {
  KeyDebouncer_loop();
}