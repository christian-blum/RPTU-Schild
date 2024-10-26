#include <Arduino.h>
#include <Preferences.h>

#include "effekt.h"
#include "led_matrix.h"
#include "text_5x7.h"
#include "defaults.h"
#include "einstellungen.h"
#include "gimp_artwork.h"
#include "cb_scheduler.h"
#include "gimp_smiley_grinsend.h"



Effekt::Effekt(bool loeschbar, bool aktiv, uint16_t gewichtung) {
  Effekt::loeschbar = loeschbar;
  Effekt::aktiv = Effekt::default_aktiv = aktiv;
  Effekt::gewichtung = Effekt::default_gewichtung = gewichtung;
}

bool Effekt::doit() {
  return true; // na los, lach wieder, Compiler!
}



/*
struct sLaufschrift *els;


bool effekt_laufschrift() {
  bool fertig = laufschrift_rendern(els);
  if (fertig) {
    free(els);
    els = NULL;
  }
  return fertig;
}

*/


bool effekt_laufschrift_credits() {
  if (!els) {
    els = (struct sLaufschrift *) malloc(sizeof(struct sLaufschrift));
    memset(els, 0, sizeof(sLaufschrift));
    els->text = credits;
    els->y = (LED_COUNT_Y - 7) / 2;
    els->schriftfarbe.x = LAUFSCHRIFT_CREDITS_SCHRIFTFARBE;
    els->hintergrundfarbe.x = LAUFSCHRIFT_CREDITS_HINTERGRUNDFARBE;
    els->millis = laufschrift_delay;
  }
  return effekt_laufschrift();
}

bool effekt_laufschrift_releaseInfo() {
  if (!els) {
    els = (struct sLaufschrift *) malloc(sizeof(struct sLaufschrift));
    memset(els, 0, sizeof(struct sLaufschrift));
    els->text = releaseInfo;
    els->y = (LED_COUNT_Y - 7);
    els->schriftfarbe.x = LAUFSCHRIFT_RELEASEINFO_SCHRIFTFARBE;
    els->hintergrundfarbe.x = LAUFSCHRIFT_RELEASEINFO_HINTERGRUNDFARBE;
    els->millis = laufschrift_delay;
  }
  return effekt_laufschrift();
}





struct sEffektDef {
  const char *name;
  bool aktiv;
  uint16_t gewichtung;
  bool (*funktion)();
};

struct sEffekt {
  bool aktiv;
  uint16_t gewichtung;
  bool (*funktion)();
};

const struct sEffektDef effekte_defaults[] = {
  "Laufschrift Version", false,  100, effekt_laufschrift_releaseInfo,  // er wird aber beim Booten getriggert
  "Laufschrift Credits", true,   100, effekt_laufschrift_credits,
//  "Grinse-Smiley",       true,   200, effekt_smiley_grinsend,
};

#define EFFEKTE_ANZAHL (sizeof(effekte_defaults) / sizeof(effekte_defaults[0]))

struct sEffekt effekte[EFFEKTE_ANZAHL];

uint32_t effekte_summe_gewichte;

void effekte_gewichtungen_summieren() {
  effekte_summe_gewichte = 0;
  for (int i = 0; i < EFFEKTE_ANZAHL; i++) {
    struct sEffekt *e;
    e = &effekte[i];
    if (e->aktiv) effekte_summe_gewichte += e->gewichtung;
  }
}



#define EFFEKTE_DELIMITER ";"

#undef DEBUG_EFFEKTE_LADEN

void effekte_laden() {
  Preferences p;
  p.begin(PREF_NAMESPACE_EFFEKTE, true);
  for (int i = 0; i < EFFEKTE_ANZAHL; i++) {
    const struct sEffektDef *d = &effekte_defaults[i];
    struct sEffekt *u = &effekte[i];
    u->aktiv = d->gewichtung;
    u->gewichtung = d->gewichtung;
    u->funktion = d->funktion;
#ifdef DEBUG_EFFEKTE_LADEN
    Serial.print("Lade Effekt "); Serial.print(d->name); Serial.print(": ");
#endif
    if (p.isKey(d->name)) {
      String s = p.getString(d->name);
      char buf[s.length()+1];
      memcpy(buf, s.c_str(), s.length());
      buf[s.length()] = '\0';
#ifdef DEBUG_EFFEKTE_LADEN
      Serial.print(buf);
#endif
      char *v = strtok(buf, EFFEKTE_DELIMITER);
      if (v) {
        u->aktiv = (*v == 1);
        v = strtok(NULL, EFFEKTE_DELIMITER);
        if (v) {
          u->gewichtung = atoi(v);
        }
        else u->aktiv = false; // irgendwas war flasch
      }
      else u->aktiv = false;
    }
#ifdef DEBUG_EFFEKTE_LADEN
    Serial.print("   "); Serial.print(u->aktiv); Serial.print(" "); Serial.print(u->gewichtung);
    Serial.println();
#endif
  }
  p.end();
  effekte_gewichtungen_summieren();
}

void effekte_speichern() {
  Preferences p;
  p.begin(PREF_NAMESPACE_EFFEKTE, false);
  for (int i = 0; i < EFFEKTE_ANZAHL; i++) {
    const struct sEffektDef *d = &effekte_defaults[i];
    const struct sEffekt *u = &effekte[i];
    String x = String(u->aktiv ? "1;":"0;");
    x += u->gewichtung;
    p.putString(d->name, x);
  }
  p.end();
}



void setup_effekte() {
  effekte_laden();
}



// immer aufrufen, wenn sich der pipelineHead geändert hat, also wenn eine erste
// Konfiguration eingereiht wurde und vorher keine da war. Auch dann, gerade ein
// dequeue stattgefunden hat.
void effekt_scheduleDequeue(uint32_t milliseconds) {
  scheduler.setMeInMilliseconds(&semaphore_naechstesEffektLayerAnzeigen, milliseconds);
}

// ruft man auf, wenn man die nächste Konfiguration bearbeiten soll (also die Semaphore gesetzt ist)
struct sBitmap *effekt_peek() {
  return effekt_pipelineHead;
}

// ruft man auf, wenn man das nächste Frame für das Effektlayer will.
// Danach schmeißt man es mit free weg. Die darin gespeicherte Bitmap wird vom ledMatrix-Code weggeworfen.
struct sBitmap *effekt_dequeue() {
  struct sBitmap *b = effekt_pipelineHead;
  if (b != NULL) {
    effekt_pipelineHead = b->naechste;
    effekt_pipeline_laenge--;
    effekt_scheduleDequeue(b->milliseconds);
  }
  if (effekt_pipelineHead == NULL) {
    effekt_pipelineTail = NULL;
  }
  return b;
}

