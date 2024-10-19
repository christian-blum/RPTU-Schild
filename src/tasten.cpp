#include <Arduino.h>

#include "tasten.h"
#include "einstellungen.h"
#include "pins.h"

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

void tasten_start() {
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