#ifndef __RPTU_STUFF_H
#define __RPTU_STUFF_H

#include <Arduino.h>

extern const uint8_t CHARSET8_R[];
extern const uint8_t CHARSET8_P[];
extern const uint8_t CHARSET8_T[];
extern const uint8_t CHARSET8_U[];


extern const CRGB FARBEN[];
extern const uint8_t rptu_anzahl_farben;

bool rptu_farbkombination_erlaubt(int hf, int sf);

#endif