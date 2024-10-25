#include <Arduino.h>

#include "effekte.h"
#include "led_matrix.h"
#include "einstellungen.h"

#include <vector>




uint16_t effekt_pipeline_laenge;
struct sBitmap *effekt_pipelineHead;
struct sBitmap *effekt_pipelineTail;



std::vector<Effekt> effekte;
uint32_t effekte_summe_gewichte;
Effekt *effekt_laufend;

void effekte_gewichte_summieren() {
  uint32_t sg = 0;
  for (std::vector<Effekt>::iterator i = effekte.begin(); i < effekte.end(); i++) {
    sg += i->gewichtung;
  }
  effekte_summe_gewichte = sg;
}

void effekt_hinzufuegen(Effekt effekt) {
  effekte.push_back(effekt);
  effekte_gewichte_summieren();
}

bool effekt_entfernen(int index) {
  Effekt *e = &effekte.at(index);
  if (e != effekt_laufend) { // nicht löschen, wenn der Effekt gerade läuft!
    effekte.erase(effekte.begin() + index);
    effekte_gewichte_summieren();
    return true;
  }
  return false;
}

void effekt_queue_bitmap(struct sBitmap *effekt) {
  if (effekt_pipelineHead == NULL) {
    effekt_pipelineHead = effekt;
    effekt_pipelineTail = effekt;
    semaphore_naechstesEffektLayerAnzeigen = true;
    effekt_pipeline_laenge = 1;
  }
  else {
    effekt_pipelineTail->naechste = effekt;
    effekt_pipelineTail = effekt;
    effekt_pipeline_laenge++;
  }
}

void effekt_schedule_pause(uint32_t milliseconds) {
  struct sBitmap *b = (struct sBitmap *) malloc(sizeof(struct sBitmap));
  memset(b, 0, sizeof(struct sBitmap));
  b->bitmap = NULL;
  b->milliseconds = milliseconds;
  effekt_queue_bitmap(b);
}

Effekt *effekt_wuerfeln() {
  if (!effekte_summe_gewichte) return nullptr;
  int32_t x = random(effekte_summe_gewichte);
  for (std::vector<Effekt>::iterator i = effekte.begin(); i < effekte.end(); i++) {
    x -= i->gewichtung;
    if (x < 0) return &effekte[0] + (i - effekte.begin());
    // seufz. Siehe https://iris.artins.org/software/converting-an-stl-vector-iterator-to-a-raw-pointer/
  }
  return nullptr;
}

void effekte_setze_laufender_effekt(int welcher) {
    effekt_laufend = &effekte[welcher];
}

void effekt_pipeline_fuellen() {
  //  if (!einaus) return;  // Wenn das Schild gerade nicht an ist, brauchen auch keine Effekte erzeugt zu werden.
  if (effekt_pipeline_laenge >= EFFEKTE_PIPELINE_MINDESTLAENGE) return;  // wir haben genug auf Halde.
  if (!effekt_laufend) {
    if (!effekte_einaus) return;
    effekt_schedule_pause(random(effekt_pause_max - effekt_pause_min) + effekt_pause_min);
    effekt_laufend = effekt_wuerfeln();
  }
  while (effekt_pipeline_laenge < EFFEKTE_PIPELINE_MAXIMALLAENGE && effekt_laufend) {
    bool abgeschlossen = effekt_laufend->doit(); // dann rufen wir die Funktion mal auf...
    if (abgeschlossen) {
      effekt_laufend = nullptr;
    }
  }
}

