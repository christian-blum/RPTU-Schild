#include <Arduino.h>

#include "led_matrix.h"
#include "defaults.h"
#include "text_5x7.h"
#include "my_scheduler.h"
#include "einstellungen.h"

#ifdef HAVE_BLUETOOTH
#include "gimp_artwork.h"
#include "bluetooth_stuff.h"
#endif

volatile bool semaphore_osd_entfernen;
volatile bool semaphore_osd_helligkeit;
volatile bool semaphore_osd_effekte;
volatile bool semaphore_bt_taste_ein_aus_pairing_clear;

bool osd_effekte_sichtbar;
volatile bool semaphore_restart;    // manche Sachen kann man nur in setup() machen...


static void zeichneAusgefuelltesRechteck(struct sCRGBA *layer, sCRGBA farbe, int16_t obenLinksX, int16_t obenLinksY, uint16_t breite, uint16_t hoehe) {
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

static void zeichneFortschrittsbalkenMitRand(struct sCRGBA *layer, struct sCRGBA farbeBalken, struct sCRGBA farbeRahmen, struct sCRGBA farbeBalkenHintergrund, int16_t obenLinksX, int16_t obenLinksY, uint16_t breite, uint16_t hoehe, float fortschritt) {
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

static void zeichneRelativindikator(char *ueberschrift, float fortschritt) {
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

static void zeichneSchalter(char *ueberschrift, bool schalter) {
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


static void ARDUINO_ISR_ATTR OSD_entfernen() { // wird nur aus dem Interrupt aufgerufen
  taskOSDentfernen = NULL;  // wird im Interrupt vom Heap entfernt, wir löschen deshalb unseren Pointer auch.
  semaphore_osd_entfernen = true;
}

static void OSD_ggf_entfernen() {
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

static void OSD_Entfernen_schedulen() {
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

static void zeigeHelligkeitAn() {
  zeichneRelativindikator((char *)"Hell", (float) helligkeit / 255);
  OSD_Entfernen_schedulen();
}

static void zeigeEffekteSchalterAn() {
  osd_effekte_sichtbar = true;
  zeichneSchalter((char *)"Eff.", effekte_einaus);
  OSD_Entfernen_schedulen();
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
#endif



void osd_loop() { 
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

#ifdef HAVE_BLUETOOTH
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
#endif
  OSD_ggf_entfernen();
}