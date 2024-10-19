
#include <Arduino.h>


#define HAVE_WEBSERVER
#undef HAVE_BLUETOOTH

#define DEBUG

#ifdef HAVE_BLUETOOTH
#define BT_DEFAULT_DEVICE_NAME "RPTU-Schild"
#endif

#include "pins.h"



#include <Preferences.h>
#ifdef HAVE_BLUETOOTH
#include <BluetoothSerial.h>
#endif

#ifdef HAVE_WEBSERVER
#include "my_webserver.h"
#include "my_wifi.h"
#include "config_pages.h"
#endif

#include "my_scheduler.h"
#include "defaults.h"
#include "led_matrix.h"
#include "effekt.h"
#include "einstellungen.h"

#include "uebergaenge.h"

#if !defined( ESP32 ) 
 	#error Nee Dude, ich glaub nicht, dass das auf was anderem als ESP32 läuft. Besorg dir einen NodeMCU.
#endif

#ifdef HAVE_BLUETOOTH
// Haben wir ein Bluetooth-Modul?
#if !defined(CONFIG_BT_ENABLED) || !defined(CONFIG_BLUEDROID_ENABLED)
#error Bluetooth ist nicht aktiviert worden. Man soll wohl irgendwie `make menuconfig` ausführen. Frag mich nicht.
#endif

// Check Serial Port Profile
#if !defined(CONFIG_BT_SPP_ENABLED)
#error Serial Port Profile for Bluetooth ist nicht verfügbar oder nicht aktiviert. Das gibt es auch nur bei ESP32.
#endif
#endif

/***********************************************************
 *                                                         *
 * Grundlegende Definitionen                               *
 *                                                         *
 ***********************************************************/



#include "rptu_stuff.h"
#include "laufschrift.h"


volatile bool semaphore_osd_entfernen;
volatile bool semaphore_osd_helligkeit;
volatile bool semaphore_osd_effekte;

volatile bool semaphore_taste_einaus;
volatile bool semaphore_taste_dunkler;
volatile bool semaphore_taste_heller;
volatile bool semaphore_taste_effekte_ein_aus;

#ifdef HAVE_BLUETOOTH
volatile bool semaphore_bt_taste_ein_aus_pairing_clear;
#endif

volatile bool semaphore_restart;    // manche Sachen kann man nur in setup() machen...

bool osd_effekte_sichtbar;

#ifdef HAVE_BLUETOOTH
BluetoothSerial SerialBT;
#endif








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


























/**************************************************************
 *                                                            *
 * TASTEN                                                     *
 *                                                            *
 **************************************************************/

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


void zeichneAusgefuelltesRechteck(struct sCRGBA *layer, sCRGBA farbe, int16_t obenLinksX, int16_t obenLinksY, uint16_t breite, uint16_t hoehe) {
  // Wir machen das Clipping vorab, dann müssen wir nicht in Schleifen testen.
  if (obenLinksX < 0) {
    breite += obenLinksX;
    obenLinksX = 0;
  }
  if (obenLinksY < 0) {
    hoehe += obenLinksY;
    obenLinksY = 0;
  }
  if (obenLinksX + breite >= LED_COUNT_X) {
    breite = LED_COUNT_X - obenLinksX;
  }
  if (breite <= 0) return;
  if (obenLinksY + hoehe >= LED_COUNT_Y) {
    hoehe = LED_COUNT_Y - obenLinksY;
  }
  if (hoehe <= 0) return;
  // Jetzt kann nichts mehr passieren.
  for (int16_t y = obenLinksY; y < obenLinksY + hoehe; y++) {
    for (int16_t x = obenLinksX; x < obenLinksX + breite; x++) {
      uint16_t index = y * LED_COUNT_X + x;
      layer[index] = farbe;
    }
  }
}

void zeichneFortschrittsbalkenMitRand(struct sCRGBA *layer, struct sCRGBA farbeBalken, struct sCRGBA farbeRahmen, struct sCRGBA farbeBalkenHintergrund, int16_t obenLinksX, int16_t obenLinksY, uint16_t breite, uint16_t hoehe, float fortschritt) {
  // Wir malen alles in einem :-)
  if (fortschritt < 0.0f) fortschritt = 0.0f;
  if (fortschritt > 1.0f) fortschritt = 1.0f;
  int16_t balkenX = (breite - 2 + 0.5f) * fortschritt + obenLinksX + 1;
  for (uint16_t y = obenLinksY; y < obenLinksY + hoehe; y++) {
    if (y < 0) continue;
    if (y >= LED_COUNT_Y) break;
    for (uint16_t x = obenLinksX; x < obenLinksX + breite; x++) {
      if (x < 0) continue;
      if (x >= LED_COUNT_X) break;
      struct sCRGBA farbe = farbeBalkenHintergrund;
      if (y == obenLinksY || y == obenLinksY + hoehe - 1) farbe = farbeRahmen;
      else if (x == obenLinksX || x == obenLinksX + breite - 1) farbe = farbeRahmen;
      else if (x < balkenX) farbe = farbeBalken;
      uint16_t index = y * LED_COUNT_X + x;
      layer[index] = farbe;
    }
  }
}

void zeichneRelativindikator(char *ueberschrift, float fortschritt) {
  // Das ist jetzt viel Trara, aber der Compiler ist clever, der macht das ALLES weg. Der merkt, dass am Ende eh immer nur eine konstante 32bit-Zahl steht.
  struct sCRGBA farbe_hintergrund; farbe_hintergrund.x = OSD_FARBE_HINTERGRUND;
  struct sCRGBA farbe_schrift; farbe_schrift.x = OSD_FARBE_SCHRIFT;
  struct sCRGBA farbe_rahmen; farbe_rahmen.x = OSD_FARBE_RAHMEN;
  struct sCRGBA farbe_balken; farbe_balken.x = OSD_FARBE_BALKEN;
  struct sCRGBA farbe_balkenhintergrund; farbe_balkenhintergrund.x = OSD_FARBE_BALKENHINTERGRUND;

  struct sCRGBA *bitmap = (struct sCRGBA *) calloc(LED_COUNT, sizeof(struct sCRGBA));
  memset(bitmap, 0, sizeof(struct sCRGBA) * LED_COUNT);
  zeichneAusgefuelltesRechteck(bitmap, farbe_hintergrund, 1, 8, 30, 15);
  char *c = ueberschrift;
  int16_t zeichenzahl = 0;
  while (*(c++)) zeichenzahl++;
  uint16_t textbreite = 6 * zeichenzahl - 1;
  struct sPosition textpos;
  textpos.x = 15 - textbreite / 2;
  textpos.y = 9;
  text_rendern(bitmap, textpos, farbe_schrift, farbe_hintergrund, ueberschrift);
  zeichneFortschrittsbalkenMitRand(bitmap, farbe_balken, farbe_rahmen, farbe_balkenhintergrund, 2, 17, 28, 4, fortschritt);
  ledMatrix_updateLayer(LAYER_OSD, bitmap); // bitmap wird davon freigegeben
}



void zeichneSchalter(char *ueberschrift, bool schalter) {
  struct sCRGBA farbe_hintergrund; farbe_hintergrund.x = OSD_FARBE_HINTERGRUND;
  struct sCRGBA farbe_schrift; farbe_schrift.x = OSD_FARBE_SCHRIFT;
  struct sCRGBA farbe_rahmen; farbe_rahmen.x = OSD_FARBE_RAHMEN;
  struct sCRGBA farbe_balken; farbe_balken.x = OSD_FARBE_BALKEN;
  struct sCRGBA farbe_balkenhintergrund; farbe_balkenhintergrund.x = OSD_FARBE_BALKENHINTERGRUND;

  struct sCRGBA *bitmap = (struct sCRGBA *) calloc(LED_COUNT, sizeof(struct sCRGBA));
  memset(bitmap, 0, sizeof(struct sCRGBA) * LED_COUNT);
  zeichneAusgefuelltesRechteck(bitmap, farbe_hintergrund, 1, 8, 30, 15);
  char *c = ueberschrift;
  int16_t zeichenzahl = 0;
  while (*(c++)) zeichenzahl++;
  uint16_t textbreite = 6 * zeichenzahl - 1;
  struct sPosition textpos;
  textpos.x = 15 - textbreite / 2;
  textpos.y = 9;
  text_rendern(bitmap, textpos, farbe_schrift, farbe_hintergrund, ueberschrift);

  float fortschritt;
  if (!schalter) {
    farbe_balken.x = OSD_FARBE_INAKTIV;
    fortschritt = 0.4f;
  }
  else {
    fortschritt = 0.6f;
    farbe_balken = farbe_balkenhintergrund;
    farbe_balkenhintergrund.x = OSD_FARBE_AKTIV;
  }

  zeichneFortschrittsbalkenMitRand(bitmap, farbe_balken, farbe_rahmen, farbe_balkenhintergrund, 10, 17, 12, 4, fortschritt);
  ledMatrix_updateLayer(LAYER_OSD, bitmap); // bitmap wird davon freigegeben
}



struct sTask * volatile taskOSDentfernen;


void ARDUINO_ISR_ATTR OSD_entfernen() { // wird nur aus dem Interrupt aufgerufen
  taskOSDentfernen = NULL;  // wird im Interrupt vom Heap entfernt, wir löschen deshalb unseren Pointer auch.
  semaphore_osd_entfernen = true;
}

void OSD_ggf_entfernen() {
  if (semaphore_osd_entfernen) {
    semaphore_osd_entfernen = false;
    osd_effekte_sichtbar = false;
    ledMatrix_updateLayer(LAYER_OSD, NULL);
    semaphore_ledMatrix_update = true;
    if (semaphore_restart) {
      ledMatrix_aus();
      ESP.restart();
    }
  }
}

void OSD_Entfernen_schedulen() {
  ATOMIC() {
    if (taskOSDentfernen == NULL) {
      taskOSDentfernen = (struct sTask *) malloc(sizeof(struct sTask));
      memset(taskOSDentfernen, 0, sizeof(struct sTask));
      taskOSDentfernen->function = &OSD_entfernen;
      scheduleIn(taskOSDentfernen, OSD_DAUER);
    }
    else {
      rescheduleIn(taskOSDentfernen, OSD_DAUER);
    }
  }
}

void zeigeHelligkeitAn() {
  zeichneRelativindikator((char *)"Hell", (float) helligkeit / 255);
  OSD_Entfernen_schedulen();
}

void zeigeEffekteSchalterAn() {
  osd_effekte_sichtbar = true;
  zeichneSchalter((char *)"Eff.", effekte_einaus);
  OSD_Entfernen_schedulen();
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
  wifi_clearPreferences();
  webserver_clearPreferences();
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
#endif
  srand(generateRandomSeed());
  preferences_laden();
  ledMatrix_start();
  scheduler_start();
  tasten_start();
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
  effekte_setze_laufender_effekt(0); // release info - dieser Hack gefällt mir gar nicht
  show_chip_data();
}



void tasten() {
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
        struct sTask *t = (struct sTask *) malloc(sizeof(struct sTask));
        memset(t, 0, sizeof(struct sTask));
        t->semaphore = &semaphore_taste_heller;
        scheduleIn(t, TASTENWIEDERHOLZEIT);
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
        struct sTask *t = (struct sTask *) malloc(sizeof(struct sTask));
        memset(t, 0, sizeof(struct sTask));
        t->semaphore = &semaphore_taste_dunkler;
        scheduleIn(t, TASTENWIEDERHOLZEIT);
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


#ifdef HAVE_BLUETOOTH

void OSD_bluetooth_zustand(char *text, struct sCRGBA farbe_schrift) {
  struct sCRGBA bt_farbe_hintergrund; bt_farbe_hintergrund.x = BT_FARBE_HINTERGRUND;
  struct sCRGBA *bitmap = (struct sCRGBA *) calloc(LED_COUNT, sizeof(struct sCRGBA));
  memset(bitmap, 0, sizeof(struct sCRGBA) * LED_COUNT);

  zeichneAusgefuelltesRechteck(bitmap, bt_farbe_hintergrund, 1, 2, 30, 15);
  gimp_rendern(bitmap, &gimp_bluetooth_logo, 2, 3, REFPUNKT_OBEN_LINKS, 256);
  struct sPosition textpos;
  textpos.x = 11;
  textpos.y = 5;
  text_rendern(bitmap, textpos, farbe_schrift, bt_farbe_hintergrund, text);
  ledMatrix_updateLayer(LAYER_OSD, bitmap); // bitmap wird davon freigegeben
  OSD_Entfernen_schedulen();
}


void BT_neu_pairen() {
  SerialBT.deleteAllBondedDevices();
}



void BT_setup() {
  // Wir sparen uns den Authentisierungs-Schnickschnack und machen das auf der höheren Ebene.
  // Wie man SSP macht, findest du hier: https://github.com/espressif/arduino-esp32/tree/master/libraries/BluetoothSerial/examples
}


uint8_t osd_bt_level;
#define BT_BUFFER_SIZE 128
char bt_buffer[BT_BUFFER_SIZE];
int bt_buffer_inptr;

void bt_setup() {
  if (bt_einaus) SerialBT.begin(bt_device_name);
  if (bt_einaus == 2) {
    SerialBT.deleteAllBondedDevices();
    bt_einaus = 0;
    preferences_speichern = true;
  }
  SerialBT.register_callback(bt_callback);
}



//typedef enum {
//    ESP_SPP_INIT_EVT                    = 0,                /*!< When SPP is initialized, the event comes */
//    ESP_SPP_UNINIT_EVT                  = 1,                /*!< When SPP is deinitialized, the event comes */
//    ESP_SPP_DISCOVERY_COMP_EVT          = 8,                /*!< When SDP discovery complete, the event comes */
//    ESP_SPP_OPEN_EVT                    = 26,               /*!< When SPP Client connection open, the event comes */
//    ESP_SPP_CLOSE_EVT                   = 27,               /*!< When SPP connection closed, the event comes */
//    ESP_SPP_START_EVT                   = 28,               /*!< When SPP server started, the event comes */
//    ESP_SPP_CL_INIT_EVT                 = 29,               /*!< When SPP client initiated a connection, the event comes */
//    ESP_SPP_DATA_IND_EVT                = 30,               /*!< When SPP connection received data, the event comes, only for ESP_SPP_MODE_CB */
//    ESP_SPP_CONG_EVT                    = 31,               /*!< When SPP connection congestion status changed, the event comes, only for ESP_SPP_MODE_CB */
//    ESP_SPP_WRITE_EVT                   = 33,               /*!< When SPP write operation completes, the event comes, only for ESP_SPP_MODE_CB */
//    ESP_SPP_SRV_OPEN_EVT                = 34,               /*!< When SPP Server connection open, the event comes */
//    ESP_SPP_SRV_STOP_EVT                = 35,               /*!< When SPP server stopped, the event comes */
//    ESP_SPP_VFS_REGISTER_EVT            = 36,               /*!< When SPP VFS register, the event comes */
//    ESP_SPP_VFS_UNREGISTER_EVT          = 37,               /*!< When SPP VFS unregister, the event comes */
//} esp_spp_cb_event_t;



void bt_callback(esp_spp_cb_event_t event, esp_spp_cb_param_t *param) {
  if (event == ESP_SPP_SRV_OPEN_EVT) { // Verbindungsaufbau durch die Client-App
    bt_clear();
  }
  else if (event == ESP_SPP_DATA_IND_EVT) { // Client-App hat Daten geschickt
    while (SerialBT.available()) {
      char c = SerialBT.read();
      if (c == '\n') {}
      else if (c == '\r') {
        bt_buffer[bt_buffer_inptr] = '\0';
        bt_command(bt_buffer);
        bt_buffer_inptr = 0;
      }
      else if (bt_buffer_inptr < BT_BUFFER_SIZE - 1) bt_buffer[bt_buffer_inptr++] = c;
    }
  }
  else {
    Serial.print("??? Unknown callback event "); Serial.print(event); Serial.println();
  }
}


void bt_reply(char *reply) {
  SerialBT.write((const uint8_t *)reply, strlen(reply));
  SerialBT.write('\r');
  SerialBT.write('\n');
  SerialBT.flush();
}

#define BT_OK_TAG "OK"
void bt_reply_ok() {
  bt_reply(BT_OK_TAG);
}

#define BT_ERR_TAG "ERR "
void bt_reply_err(char *desc) {
  SerialBT.write((const uint8_t *)BT_ERR_TAG, sizeof(BT_ERR_TAG));
  SerialBT.write((const uint8_t *)desc, strlen(desc));
  SerialBT.write('\r');
  SerialBT.write('\n');
  SerialBT.flush();
}


#define SEP " \t"

void bt_command(char *cmd) {
#ifdef DEBUG
  Serial.print("> ");
  Serial.println(cmd);
#endif
  char *command = strtok(cmd, SEP);
  if (!strcmp("set", command)) {
    char *arg1 = strtok(NULL, SEP);
    if (strcmp(PREF_HELLIGKEIT, arg1)) {
      char *arg2 = strtok(NULL, SEP);
      int x = strtol(arg2, NULL, 10);
      if (x >= 0 || x <= 255) { 
        helligkeit = x;
        preferences_speichern = true;
        bt_reply_ok();
      }
      else {
        bt_reply_err("Wert unzulässig");
      }
    }
    else if (strcmp(PREF_BT_DEVICE_NAME, arg1)) {
      char *arg2 = strtok(NULL, NULL);
      bt_device_name = arg2;
      bt_reply_ok();
    }
    else {
      bt_reply_err("Unbekannter Parameter");
    }
  }
  else if (!strcmp("get", command)) {
    char *arg1 = strtok(NULL, SEP);
    if (strcmp(PREF_HELLIGKEIT, arg1)) {
      char num[4];
      sprintf(num, "%d", helligkeit);
      bt_reply(num);
    }
    else if (strcmp(PREF_BT_DEVICE_NAME, arg1)) {
      char buf[32];
      bt_device_name.toCharArray(buf, sizeof(buf)-1);
      buf[sizeof(buf)-1] = 0;
      bt_reply(buf);
    }
    else {
      bt_reply_err("Unbekannter Parameter");
    }
  }
  else if (!strcmp("inc", command)) {
    char *arg1 = strtok(NULL, SEP);
    if (strcmp(PREF_HELLIGKEIT, arg1)) {
      if (helligkeit < 255) {
        helligkeit++;
        preferences_speichern = true;
      }
      char num[4];
      sprintf(num, "%d", helligkeit);
      bt_reply(num);
    }
    else {
      bt_reply_err("Unbekannter Parameter");
    }
  }
  else if (!strcmp("dec", command)) {
    char *arg1 = strtok(NULL, SEP);
    if (strcmp(PREF_HELLIGKEIT, arg1)) {
      if (helligkeit > 0) {
        helligkeit--;
        preferences_speichern = true;
      }
      char num[4];
      sprintf(num, "%d", helligkeit);
      bt_reply(num);
    }
    else {
      bt_reply_err("Unbekannter Parameter");
    }
  }
  else {
    bt_reply_err("Unbekanner Befehl");
  }
}

void bt_clear() {
  Serial.println("*** BT connect ***");
}

void loop_bluetooth() {
  struct sCRGBA farbe_schrift;
  if (semaphore_bt_taste_ein_aus_pairing_clear) {
    semaphore_bt_taste_ein_aus_pairing_clear = false;
    osd_bt_level++;
    if (osd_bt_level == 2) {
      bt_einaus = !bt_einaus;
      preferences_speichern = true;
      semaphore_restart = true;
    }
    if (osd_bt_level <= 2) {
      char *text = (char *) (bt_einaus ? "ein" : "aus");
      farbe_schrift.x = bt_einaus ? BT_FARBE_EIN : BT_FARBE_AUS;
      OSD_bluetooth_zustand(text, farbe_schrift);
    }
    if (osd_bt_level == 3) {
      char *text = "del";
      farbe_schrift.x = BT_FARBE_DEL;
      OSD_bluetooth_zustand(text, farbe_schrift);
      bt_einaus = 2;  // magic cookie...
      preferences_speichern = true;
      semaphore_restart = true;
    }
  }
}

#endif


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
  tasten();
  if (einaus) {
    if (istAus) istAus = false;

    base_pipeline_fuellen();
    effekt_pipeline_fuellen();

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

    if (semaphore_osd_entfernen) {
      semaphore_osd_entfernen = false;
      ledMatrix_updateLayer(LAYER_OSD, NULL);
      semaphore_ledMatrix_update = true;
#ifdef HAVE_BLUETOOTH
      osd_bt_level = 0;
#endif
    }

    if (semaphore_osd_helligkeit) {
      semaphore_osd_helligkeit = false;
      zeigeHelligkeitAn();
    }

    if (semaphore_osd_effekte) {
      semaphore_osd_effekte = false;
      zeigeEffekteSchalterAn();
    }

    OSD_ggf_entfernen();

    // davor gegebenenfalls Overlays rendern - die setzen dieselbe Semaphore
    if (semaphore_ledMatrix_update) {
#ifdef FRAMERATE_MAX
      uint64_t stand = timerRead(scheduler_timer);
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
  loop_bluetooth();
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
