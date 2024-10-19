#include <Arduino.h>
#include <FastLED.h>
#include "rptu_stuff.h"

const uint8_t CHARSET8_R[] = {
  0b11111100,
  0b10000010,
  0b10000010,
  0b10000010,
  0b11111100,
  0b10001000,
  0b10000100,
  0b10000010,
};

const uint8_t CHARSET8_P[] = {
  0b11111100,
  0b10000010,
  0b10000010,
  0b10000010,
  0b11111100,
  0b10000000,
  0b10000000,
  0b10000000,
};

const uint8_t CHARSET8_T[] = {
  0b11111110,
  0b00010000,
  0b00010000,
  0b00010000,
  0b00010000,
  0b00010000,
  0b00010000,
  0b00010000,
};

const uint8_t CHARSET8_U[] = {
  0b10000010,
  0b10000010,
  0b10000010,
  0b10000010,
  0b10000010,
  0b10000010,
  0b01000100,
  0b00101000,
};


const CRGB FARBEN[] = {
  CRGB(0x507289), // blaugrau
  CRGB(0x77b6ba), // grüngrau
  CRGB(0x042c58), // dunkelblau
  CRGB(0x6ab2e7), // hellblau
  CRGB(0x006b6b), // dunkelgrün
  CRGB(0x26d0c7), // hellgrün
  CRGB(0x4c3575), // violett
  CRGB(0xd13896), // pink
  CRGB(0xe31b4c), // rot
  CRGB(0xffa252), // orange
  CRGB(0x000000), // schwarz
  CRGB(0xffffff), // weiß
};

const uint8_t rptu_anzahl_farben = sizeof(FARBEN)/sizeof(FARBEN[0]);


const static uint8_t KOMBINATION_ERLAUBT[][rptu_anzahl_farben] = {
  0, 0, 1, 1, 1, 0, 1, 1, 1, 1, 1, 1, // blaugrau
  0, 0, 1, 1, 1, 0, 1, 1, 1, 1, 1, 1, // grüngrau
  1, 1, 0, 1, 1, 1, 1, 1, 1, 1, 0, 1, // dunkelblau
  1, 1, 0, 0, 1, 1, 1, 1, 1, 0, 1, 1, // hellblau
  1, 1, 1, 1, 0, 0, 1, 1, 1, 1, 0, 1, // dunkelgrün
  0, 0, 1, 1, 0, 0, 1, 1, 1, 1, 1, 1, // hellgrün
  1, 1, 1, 1, 0, 0, 1, 1, 1, 1, 0, 1, // violett
  1, 1, 1, 1, 1, 1, 0, 0, 0, 1, 1, 1, // pink
  1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 1, 1, // rot
  1, 1, 1, 0, 1, 1, 1, 1, 0, 0, 1, 1, // orange
  1, 1, 0, 1, 0, 1, 0, 1, 1, 1, 0, 1, // schwarz
  1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, // weiß
};

bool rptu_farbkombination_erlaubt(int hf, int sf) {
  return KOMBINATION_ERLAUBT[hf][sf];
}