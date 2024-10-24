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

volatile bool semaphore_taste_einaus;
volatile bool semaphore_taste_dunkler;
volatile bool semaphore_taste_heller;
volatile bool semaphore_taste_effekte_ein_aus;


void ARDUINO_ISR_ATTR ISR_taste_ein_aus() {
  semaphore_taste_einaus = true;
}

void ARDUINO_ISR_ATTR ISR_taste_heller() {
  if (!einaus) return;
  semaphore_taste_heller = true;
}

void ARDUINO_ISR_ATTR ISR_taste_dunkler() {
  if (!einaus) return;
  semaphore_taste_dunkler = true;
}

void ARDUINO_ISR_ATTR ISR_taste_effekte_ein_aus() {
  if (!einaus) return;
  semaphore_taste_effekte_ein_aus = true;
}

#ifdef HAVE_BLUETOOTH
void ARDUINO_ISR_ATTR ISR_taste_bt_ein_aus_pairing_clear() {
  if (!einaus) return;
  semaphore_bt_taste_ein_aus_pairing_clear = true;
}
#endif

void tasten_setup() {
  pinMode(EINAUS_PIN, INPUT);
  pinMode(HELLER_PIN, INPUT);
  pinMode(DUNKLER_PIN, INPUT);
  pinMode(EFFEKTE_EINAUS_PIN, INPUT);
#ifdef HAVE_BLUETOOTH
  pinMode(BT_EINAUSPAIRINGCLEAR_PIN, INPUT);
#endif
  attachInterrupt(digitalPinToInterrupt(EINAUS_PIN), ISR_taste_ein_aus, RISING);
  attachInterrupt(digitalPinToInterrupt(HELLER_PIN), ISR_taste_heller, RISING);
  attachInterrupt(digitalPinToInterrupt(DUNKLER_PIN), ISR_taste_dunkler, RISING);
  attachInterrupt(digitalPinToInterrupt(EFFEKTE_EINAUS_PIN), ISR_taste_effekte_ein_aus, RISING);
#ifdef HAVE_BLUETOOTH
  attachInterrupt(digitalPinToInterrupt(BT_EINAUSPAIRINGCLEAR_PIN), ISR_taste_bt_ein_aus_pairing_clear, RISING);
#endif
}


void tasten_loop() {
  if (semaphore_taste_einaus) {
    semaphore_taste_einaus = false;
    einaus = !einaus;
    preferences_speichern = true;
  }
  if (semaphore_taste_heller) {
    semaphore_taste_heller = false;
    semaphore_osd_helligkeit = true;
    if (digitalRead(HELLER_PIN)) {
      if (helligkeit < HELLIGKEIT_MAX) {
        helligkeit++;
        scheduler.setMeInMilliseconds(&semaphore_taste_heller, TASTENWIEDERHOLZEIT);
        semaphore_ledMatrix_update = true;
        preferences_speichern = true;
      }
    }
  }
  if (semaphore_taste_dunkler) {
    semaphore_taste_dunkler = false;
    semaphore_osd_helligkeit = true;
    if (digitalRead(DUNKLER_PIN)) {
      if (helligkeit > HELLIGKEIT_MIN) {
        helligkeit--;
        scheduler.setMeInMilliseconds(&semaphore_taste_dunkler, TASTENWIEDERHOLZEIT);
        semaphore_ledMatrix_update = true;
        preferences_speichern = true;
      }
    } 
  }
  if (semaphore_taste_effekte_ein_aus) {
    semaphore_osd_effekte = true;
    semaphore_taste_effekte_ein_aus = false;
    if (osd_effekte_sichtbar) {
      effekte_einaus = !effekte_einaus;
    }
  }
}

