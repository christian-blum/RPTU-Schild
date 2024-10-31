#include <Arduino.h>

#include "effekte.h"
#include "led_matrix.h"
#include "einstellungen.h"
#include "gimp_smiley_grinsend.h"
#include "effekt_gimp.h"
#include "effekt_laufschrift.h"
#include "effekt_pause.h"

#include <vector>




uint16_t effekt_pipeline_laenge;
struct sBitmap *effekt_pipelineHead;
struct sBitmap *effekt_pipelineTail;

struct sCRGBA ls_releaseInfo_hintergrundfarbe = { .x = 0xff200000 };
struct sCRGBA ls_releaseInfo_schriftfarbe = { .x = 0xff00c0c0 };
struct sCRGBA ls_credits_hintergrundfarbe = { .x = 0xfc000000 };
struct sCRGBA ls_credits_schriftfarbe = { .x = 0xfc00c000 };

Effekt_Laufschrift effekt_laufschrift_releaseInfo(false, true, 100, "el_swver", "Laufschrift SW Version", releaseInfo, 17, 50, ls_releaseInfo_schriftfarbe, ls_releaseInfo_hintergrundfarbe);
Effekt_Laufschrift effekt_laufschrift_credits(false, true, 100,  "el_credits", "Laufschrift Credits", credits, 8, 50, ls_credits_schriftfarbe, ls_credits_hintergrundfarbe);
Effekt_GIMP effekt_smiley_grinsend(false, true, 100, &gimp_smiley_grinsend, 4000);

std::array<Effekt *, 3> effekte_prototypen = {
  &effekt_laufschrift_releaseInfo,
  &effekt_laufschrift_credits,
  &effekt_smiley_grinsend,
};

std::vector<Effekt *> effekte;
uint32_t effekte_summe_gewichte;
Effekt *effekt_laufend;

Effekt_Pause effekt_pause;

void effekte_gewichtungen_summieren() {
  uint32_t sg = 0;
  for (std::vector<Effekt *>::iterator i = effekte.begin(); i < effekte.end(); i++) {
    sg += (*i)->gewichtung;
  }
  effekte_summe_gewichte = sg;
}

void effekt_hinzufuegen(Effekt *effekt) {
  effekte.push_back(effekt);
  effekte_gewichtungen_summieren();
}

bool effekt_entfernen(int index) {
  Effekt *e = effekte.at(index);
  if (e != effekt_laufend) { // nicht löschen, wenn der Effekt gerade läuft!
    effekte.erase(effekte.begin() + index);
    effekte_gewichtungen_summieren();
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

Effekt *effekt_wuerfeln() {
  if (!effekte_summe_gewichte) return nullptr;
  int32_t x = random(effekte_summe_gewichte);
  for (int i = 0; i < effekte.size(); i++) {
    Effekt *e = effekte[i];
    x -= e->gewichtung;
    if (x < 0) {
      return e;
    }
  }
  return nullptr;
}

void effekte_setze_laufender_effekt(int welcher) {
    effekt_laufend = effekte[welcher];
}

bool wuerfeln = false;

void effekte_pipeline_fuellen() {
  //  if (!einaus) return;  // Wenn das Schild gerade nicht an ist, brauchen auch keine Effekte erzeugt zu werden.
  if (effekt_pipeline_laenge >= EFFEKTE_PIPELINE_MINDESTLAENGE) return;  // wir haben genug auf Halde.
  if (!effekt_laufend && wuerfeln) {
    if (!effekte_einaus) return;
    effekt_laufend = effekt_wuerfeln();
    wuerfeln = false;
  }
  while (effekt_pipeline_laenge < EFFEKTE_PIPELINE_MAXIMALLAENGE && effekt_laufend) {
    bool abgeschlossen = effekt_laufend->doit();
    if (abgeschlossen) {
      if (!wuerfeln) {
        effekt_laufend = &effekt_pause;
        wuerfeln = true;
      }
      else {
        effekt_laufend = nullptr;
      }
    }
  }
}

void effekte_prefs_schreiben() {
  for (int i = 0; i < effekte.size(); i++) {
    effekte[i]->prefs_schreiben();
  }
  effekte_gewichtungen_summieren(); // das ergibt tatsächlich Sinn.
}

void effekte_prefs_laden() {
  for (int i = 0; i < effekte.size(); i++) {
    effekte[i]->prefs_laden();
  }
  effekte_gewichtungen_summieren();
}

void setup_effekte() {
  for (int i = 0; i < effekte_prototypen.size(); i++) {
    effekt_hinzufuegen(effekte_prototypen[i]);
  }
  effekte_prefs_laden();
}