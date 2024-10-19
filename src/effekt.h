#ifndef __EFFEKT_H
#define __EFFEKT_H

extern uint16_t effekt_pipeline_laenge;
extern struct sBitmap *effekt_pipelineHead;
extern struct sBitmap *effekt_pipelineTail;


void setup_effekte();
void effekt_queue(struct sBitmap *effekt);
void effekt_schedule_pause(uint32_t milliseconds);
void effekte_setze_laufender_effekt(int welcher); // sollte es echt nicht geben...
void effekt_pipeline_fuellen();
struct sBitmap *effekt_dequeue();

#endif