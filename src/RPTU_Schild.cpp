
#include <Arduino.h>
#include "defaults.h"
#include "backdoor.h"


#define DEBUG


#include "pins.h"



#include <Preferences.h>

#ifdef HAVE_WEBSERVER
#include "my_webserver.h"
#include "my_wifi.h"
#include "config_pages.h"
#endif

#include "cb_scheduler.h"
#include "led_matrix.h"
#include "effekte.h"
#include "einstellungen.h"
#include "tasten.h"
#include "osd.h"
#include "uebergaenge.h"
#include "rptu_stuff.h"
#include "tasten.h"



#if !defined( ESP32 ) 
 	#error Nee Dude, ich glaub nicht, dass das auf was anderem als ESP32 läuft. Besorg dir einen NodeMCU.
#endif

#ifdef HAVE_BLUETOOTH
#include "bluetooth_stuff.h"
// Haben wir ein Bluetooth-Modul?
#if !defined(CONFIG_BT_ENABLED) || !defined(CONFIG_BLUEDROID_ENABLED)
#error Bluetooth ist nicht aktiviert worden. Man soll wohl irgendwie `make menuconfig` ausführen. Frag mich nicht.
#endif

// Check Serial Port Profile
#if !defined(CONFIG_BT_SPP_ENABLED)
#error Serial Port Profile for Bluetooth ist nicht verfügbar oder nicht aktiviert. Das gibt es auch nur bei ESP32.
#endif
#endif





CB_Scheduler scheduler;







/******************************************************************
 *                                                                *
 * Diverser Kleinkram                                             *
 *                                                                *
 ******************************************************************/

 
/*
 * Arduino initialisiert den Zufallszahlengenerator einfach nicht gut. Wir machen es besser.
 * Wir nehmen uns einen der Analogeingänge und verwenden das niedrigste Bit.
 * Die Idee stammt von: https://rheingoldheavy.com/better-arduino-random-values/
 */
uint32_t generateRandomSeed() {
  uint8_t  seedBitValue  = 0;
  uint8_t  seedByteValue = 0;
  uint32_t seedWordValue = 0;

  for (uint8_t wordShift = 0; wordShift < 4; wordShift++) {
    for (uint8_t byteShift = 0; byteShift < 8; byteShift++) {
      for (uint8_t bitSum = 0; bitSum <= 8; bitSum++) {
        seedBitValue = seedBitValue + (analogRead(SEED_PIN) & 0x01);
      }
      delay(1);
      seedByteValue = seedByteValue | ((seedBitValue & 0x01) << byteShift);
      seedBitValue = 0;
    }
    seedWordValue = seedWordValue | (uint32_t)seedByteValue << (8 * wordShift);
    seedByteValue = 0;  }
  return (seedWordValue);
}



























void show_chip_data() {
  uint32_t chipId = 0;

  Serial.println("----------------------------------------------------------------");
    for (int i = 0; i < 17; i = i + 8) {
    chipId |= ((ESP.getEfuseMac() >> (40 - i)) & 0xff) << i;
  }

  Serial.printf("ESP32 Chip model = %s Rev %d\n", ESP.getChipModel(), ESP.getChipRevision());
  Serial.printf("This chip has %d cores\n", ESP.getChipCores());
  Serial.print("Chip ID: ");
  Serial.println(chipId);
  Serial.println("----------------------------------------------------------------");
}

void zuruecksetzen() {
  Serial.println("Alles auf Anfang. Alles Gute!");
#ifdef HAVE_WEBSERVER
  wifi_clearPreferences();
  webserver_clearPreferences();
#endif
  preferences_loeschen();
  esp_restart();
}

// Wenn beim Spannung einschalten der Ein/Aus-Knopf eine Sekunde lang gehalten wird,
// setze das Schild komplett zurück, indem alle Preferences gelöscht werden.
// Erst entprellen, dann eine Sekunde lang prüfen.
void test_reset() {
  int count = 0;
  bool x = digitalRead(EINAUS_PIN);
  while (count < 10) {
    delay(10);
    if (digitalRead(EINAUS_PIN) != x) {
      if (!x) x = true;
    }
    count++;
  }
  count = 0;
  while (count < 100 && x) {
    delay(10);
    if (!digitalRead(EINAUS_PIN)) x = false;
    count++;
  }
  if (x) zuruecksetzen();
}

void setup() {
#ifdef DEBUG
  Serial.begin(115200);
  while (!Serial) {}
#endif
  show_chip_data();
  srand(generateRandomSeed());
  preferences_laden();
  ledMatrix_start();
  scheduler.begin();
  tasten_setup();
  test_reset();
#ifdef HAVE_BLUETOOTH
  bt_setup();
#endif
#ifdef HAVE_WEBSERVER
  wifi_setup();
  webserver_setup();
  setup_config_pages();
#endif
  setup_uebergaenge();
  setup_effekte();
  //effekte_setze_laufender_effekt(0); // release info - dieser Hack gefällt mir gar nicht
}




static void alle_5_sekunden() {
  if (preferences_speichern) {
    preferences_speichern = false;
    preferences_schreiben();
  }
}

static uint32_t zuletzt;
static uint64_t ledMatrix_letztes_update;
static bool istAus;


void loop() {
  scheduler.loop();
  tasten_loop();
  if (einaus) {
    if (istAus) istAus = false;

    base_pipeline_fuellen();
    effekte_pipeline_fuellen();

    if (semaphore_naechsteBaseAnzeigen) {
      semaphore_naechsteBaseAnzeigen = false;
      struct sBitmap *b = base_dequeue();
      ledMatrix_updateLayer(LAYER_BASE, b != NULL ? b->bitmap : NULL); // bitmap wird davon freigegeben
      if (b != NULL) free(b);
    }

    if (semaphore_naechstesEffektLayerAnzeigen) {
      semaphore_naechstesEffektLayerAnzeigen = false;
      struct sBitmap *b = effekt_dequeue();
      ledMatrix_updateLayer(LAYER_EFFEKTE, b != NULL ? b->bitmap : NULL); // bitmap wird davon freigegeben
      if (b != NULL) free(b);
    }

    osd_loop();

    // davor gegebenenfalls Overlays rendern - die setzen dieselbe Semaphore
    if (semaphore_ledMatrix_update) {
#ifdef FRAMERATE_MAX
      uint64_t stand = timerRead(scheduler.timer);
      if (stand - ledMatrix_letztes_update >= SCHEDULER_TIMER_FREQUENZ / FRAMERATE_MAX) {
        ledMatrix_letztes_update = stand;
#endif
        semaphore_ledMatrix_update = false;
        ledMatrix_update();
#ifdef FRAMERATE_MAX
      }
#endif
    }
  }
  else {
    if (!istAus) {
      ledMatrix_aus();
      istAus = true;
    }
  }

#ifdef HAVE_BLUETOOTH
  bt_loop();
#endif

#ifdef HAVE_WEBSERVER
  webserver_loop();
  wifi_loop();
#endif

  uint32_t jetzt = millis();
  if (jetzt - zuletzt >= 5000) {
    zuletzt = jetzt;
    alle_5_sekunden();
  }

}
