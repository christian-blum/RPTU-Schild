#ifndef __BLUETOOTH_STUFF_H
#define __BLUETOOTH_STUFF_H

#include "defaults.h"
#ifdef HAVE_BLUETOOTH

#include <Arduino.h>

extern uint8_t bt_einaus;
extern String bt_device_name;
extern uint8_t osd_bt_level;

void bt_setup();
void bt_loop();

#endif
#endif