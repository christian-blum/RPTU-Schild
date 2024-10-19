#ifndef __OSD_H
#define __OSD_H

#include "defaults.h"

extern volatile bool semaphore_osd_entfernen;
extern volatile bool semaphore_osd_helligkeit;
extern volatile bool semaphore_osd_effekte;
#ifdef HAVE_BLUETOOTH
extern volatile bool semaphore_bt_taste_ein_aus_pairing_clear;
#endif

extern bool osd_effekte_sichtbar;
extern volatile bool semaphore_restart;    // manche Sachen kann man nur in setup() machen...

void osd_loop();

#endif