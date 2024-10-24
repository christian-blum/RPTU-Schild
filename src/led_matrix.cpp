#include <Arduino.h>
#include "led_matrix.h"
#include "einstellungen.h"
#include "pins.h"
#include "cb_scheduler.h"
#include "effekt.h"


uint16_t base_pipeline_laenge;
struct sBitmap *base_pipelineHead;
struct sBitmap *base_pipelineTail;

volatile bool semaphore_naechsteBaseAnzeigen; // muss volatile sein, weil sie im Interrupt gesetzt wird
volatile bool semaphore_naechstesEffektLayerAnzeigen;
bool semaphore_ledMatrix_update;


// immer aufrufen, wenn sich der pipelineHead geändert hat, also wenn eine erste
// Konfiguration eingereiht wurde und vorher keine da war. Auch dann, gerade ein
// dequeue stattgefunden hat.
void base_scheduleDequeue(uint32_t milliseconds) {
  scheduler.setMeInMilliseconds(&semaphore_naechsteBaseAnzeigen, milliseconds);
}

void base_queue(struct sBitmap *base) {
  if (base_pipelineHead == NULL) {
    base_pipelineHead = base;
    base_pipelineTail = base;
    semaphore_naechsteBaseAnzeigen = true;
    base_pipeline_laenge = 1;
  }
  else {
    base_pipelineTail->naechste = base;
    base_pipelineTail = base;
    base_pipeline_laenge++;
  }
}

// ruft man auf, wenn man die nächste Konfiguration bearbeiten soll (also die Semaphore gesetzt ist)
struct sBitmap *base_peek() {
  return base_pipelineHead;
}

// ruft man auf, wenn man die Konfiguration beendet hat, also man die nächste machen könnte.
// Danach schmeißt man die Konfiguration mit free weg.
struct sBitmap *base_dequeue() {
  struct sBitmap *b = base_pipelineHead;
  if (b != NULL) {
    base_pipelineHead = b->naechste;
    base_pipeline_laenge--;
    base_scheduleDequeue(b->milliseconds);
  }
  if (base_pipelineHead == NULL) {
    base_pipelineTail = NULL;
  }
  return b;
}


CRGB leds[LED_COUNT]; // zugreifen in der Reihenfolge der Verschaltung



// Wenn du mehr brauchst, füge einfach hier NULL-Pointer hinzu. Dann existieren sie einfach.
struct sCRGBA *layers[] = {
  NULL,               // base layer
  NULL,               // Effekte-Layer
  NULL,               // OSD-Layer
};

#define N_LAYERS (sizeof(layers)/sizeof(layers[0]))


void ledMatrix_start() {
  FastLED.addLeds<LED_TYPE, LED_PIN, COLOR_ORDER>(leds, LED_COUNT).setCorrection(TypicalLEDStrip);
  FastLED.setBrightness(helligkeit);
  FastLED.clear();
  FastLED.show();
}

void ledMatrix_aus() {
  FastLED.clear();
  FastLED.show();
}

/*
 * Berechnet einen Index in eine Matrix [y][x] aus der LED-Position [i].
 * Dabei wird die etwas weirde Verschaltung der Panels aufgelöst. Daraus
 * kann man dann leicht Pointer in diese Matrix berechnen.
 */
uint16_t ledMatrix_matrixIndexAusPosition(uint16_t i) {
  uint8_t panel = i / (8 * LED_COUNT_X);
  uint16_t ii = i % (8 * LED_COUNT_X);
  uint16_t x = ii / 8;
  uint16_t y = ii % 8;
  if (x & 1) y = 7 - y;
  y += panel * 8;
  uint16_t pos = y * LED_COUNT_X + x;
  return pos;
}

inline CRGB ledMatrix_ueberlagern(CRGB base, struct sCRGBA overlay) {
  uint16_t alpha = overlay.alpha;
  // shortcuts für häufige Fälle
  if (alpha == 0) return base;
  if (alpha == 255) return overlay.crgb;
  // und die seltenen berechnen wir halt
  uint16_t invAlpha = 255 - alpha;
  CRGB x;
  x.r = (invAlpha * base.r + alpha * overlay.r) / 255;
  x.g = (invAlpha * base.g + alpha * overlay.g) / 255;
  x.b = (invAlpha * base.b + alpha * overlay.b) / 255;
  return x;
}


/*
 * Argumente sind:
 *   Pointer auf ein Array der Basis (also zweidimensionales Array der CRGB Werte, [y][x])
 */
void ledMatrix_befuellen() {
  for (uint16_t i = 0; i < LED_COUNT; i++) {
    uint16_t mi = ledMatrix_matrixIndexAusPosition(i);
    CRGB x;
    if (layers[0]) x = layers[0][mi].crgb; else x = CRGB::Black;
    for (uint8_t o = 1; o < N_LAYERS; o++) {
      CRGBA *layer = layers[o];
      if (layer) x = ledMatrix_ueberlagern(x, layer[mi]);
    }
    leds[i] = x;
  }
}

void ledMatrix_update() {
  ledMatrix_befuellen();
  FastLED.setBrightness(helligkeit);
  FastLED.show();
}

void ledMatrix_updateLayer(uint8_t layer, struct sCRGBA *pixeldaten) {
  if (layers[layer]) free(layers[layer]);
  layers[layer] = pixeldaten;
  semaphore_ledMatrix_update = 1;
}

