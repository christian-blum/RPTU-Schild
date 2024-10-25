#ifndef __EFFEKT_H
#define __EFFEKT_H

#include <Arduino.h>

extern uint16_t effekt_pipeline_laenge;
extern struct sBitmap *effekt_pipelineHead;
extern struct sBitmap *effekt_pipelineTail;


void setup_effekte();
void effekt_queue_bitmap(struct sBitmap *effekt);
void effekt_schedule_pause(uint32_t milliseconds);
void effekte_setze_laufender_effekt(int welcher); // sollte es echt nicht geben...
void effekt_pipeline_fuellen();
struct sBitmap *effekt_dequeue();


class Effekt {
  private:

  public:

    const char *tag;
    const char *name;
    const char *description;
    bool aktiv;
    uint16_t gewichtung;

    Effekt(bool aktiv, uint16_t gewichtung);

    virtual bool doit();
};

#endif