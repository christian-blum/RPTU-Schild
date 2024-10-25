#ifndef __FONT5X7_H
#define __FONT5X7_H

#include <Arduino.h>
#include "led_matrix.h"

#define TEXT_5X7_SPALTEN_PRO_ZEICHEN 6




void text_rendern(struct sCRGBA *bitmap, struct sPosition wo, struct sCRGBA *schriftfarbe, struct sCRGBA *hintergrundfarbe, const char *text);

#endif