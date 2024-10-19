#ifndef __TASTEN_H
#define __TASTEN_H

extern volatile bool semaphore_taste_einaus;
extern volatile bool semaphore_taste_dunkler;
extern volatile bool semaphore_taste_heller;
extern volatile bool semaphore_taste_effekte_ein_aus;

void tasten_setup();
void tasten_loop();

#endif