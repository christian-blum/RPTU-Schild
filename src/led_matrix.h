#ifndef __LED_MATRIX_H
#define __LED_MATRIX_H

#include <Arduino.h>
#include <FastLED.h>

// Wie viele NeoPixels haben wir?
#define LED_COUNT_X 32
#define LED_COUNT_Y 24
#define LED_COUNT (LED_COUNT_Y * LED_COUNT_X)
#define LED_PANEL_ROWS ((LED_COUNT_Y + 7) / 3)

#define LED_TYPE WS2812B
#define COLOR_ORDER GRB

#define FRAMERATE_MAX 25    // Frames pro Sekunde. Wenn keine Begrenzung gewollt, einfach nicht definieren. Theoretisch gehen 43 Frames pro Sekunde.

enum LAYERS {
  LAYER_BASE = 0,
  LAYER_EFFEKTE = 1,
  LAYER_OSD = 2
};

struct sCRGBA {
  union {
    uint32_t x;
    struct {
      union {
        uint8_t r;
        uint8_t red;
        uint8_t rot;
      };
      union {
        uint8_t g;
        uint8_t green;
        uint8_t gruen;
      };
      union {
        uint8_t b;
        uint8_t blue;
        uint8_t blau;
      };
      union {
        uint8_t a;
        uint8_t alpha;
      };
    };
    CRGB crgb;
  };
};
typedef struct sCRGBA CRGBA;

struct sPosition {
  int16_t x;
  int16_t y;
};


struct sBitmap {
  CRGBA *bitmap;
  uint32_t milliseconds;
  struct sBitmap *naechste;
};

extern uint16_t base_pipeline_laenge;
extern struct sBitmap *base_pipelineHead;
extern struct sBitmap *base_pipelineTail;


extern volatile bool semaphore_naechsteBaseAnzeigen;
extern volatile bool semaphore_naechstesEffektLayerAnzeigen;
extern bool semaphore_ledMatrix_update;



void base_queue(struct sBitmap *base);
struct sBitmap *base_dequeue();
void ledMatrix_start();
void ledMatrix_aus();
void ledMatrix_update();
void ledMatrix_updateLayer(uint8_t layer, struct sCRGBA *pixeldaten);

#endif