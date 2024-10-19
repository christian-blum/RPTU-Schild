#include <Arduino.h>
#include <Preferences.h>

#include "effekt.h"
#include "led_matrix.h"
#include "laufschrift.h"
#include "defaults.h"
#include "einstellungen.h"
#include "gimp_artwork.h"
#include "my_scheduler.h"

#define PREF_NAMESPACE_EFFEKTE "effekte"

// jede braucht locker 3 kB RAM, also Vorsicht. Zu kurz kann zu Ruckeln führen, zu lang bringt nichts außer Speicherverbrauch.
#define EFFEKTE_PIPELINE_MINDESTLAENGE 2
#define EFFEKTE_PIPELINE_MAXIMALLAENGE 5


uint16_t effekt_pipeline_laenge;
struct sBitmap *effekt_pipelineHead;
struct sBitmap *effekt_pipelineTail;






void effekt_queue(struct sBitmap *effekt) {
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




struct sLaufschrift *els;


bool effekt_laufschrift() {
  bool fertig = laufschrift_rendern(els);
  if (fertig) {
    free(els);
    els = NULL;
  }
  return fertig;
}




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

enum eRefpunkt {
  REFPUNKT_OBEN_LINKS,
  REFPUNKT_MITTE
};

// alpha_faktor: 0 - 256. 256 heißt: Daten so wie im Bild angegeben. Weniger heißt: durchscheinender.
void gimp_rendern(struct sCRGBA *bitmap, const struct sGIMP *gimp, int16_t ofsx, int16_t ofsy, uint8_t refpunkt, uint16_t alpha_faktor) {
  int16_t origin_x = 0;
  int16_t origin_y = 0;
  switch (refpunkt) {
  case REFPUNKT_OBEN_LINKS:
    origin_x = ofsx;
    origin_y = ofsy;
    break;
  case REFPUNKT_MITTE:
    origin_x = ofsx - gimp->width / 2;
    origin_y = ofsy - gimp->height / 2;
    break;
  }
  const char *c = gimp->data;
  for (int16_t y = origin_y; y < origin_y + gimp->height; y++) {
    for (int16_t x = origin_x; x < origin_x + gimp->width; x++) {
      struct sCRGBA pixel;
      pixel.r = (uint8_t) *(c++);
      pixel.g = (uint8_t) *(c++);
      pixel.b = (uint8_t) *(c++);
      pixel.alpha = ((uint16_t) *(c++)) * alpha_faktor / 256;
      if (x >= 0 && x < LED_COUNT_X && y >= 0 && y < LED_COUNT_Y) {
        bitmap[y * LED_COUNT_X + x] = pixel;
      }
    }
  }
}





void effekt_schedule_pause(uint32_t milliseconds) {
  struct sBitmap *b = (struct sBitmap *) malloc(sizeof(struct sBitmap));
  memset(b, 0, sizeof(struct sBitmap));
  b->bitmap = NULL;
  b->milliseconds = milliseconds;
  effekt_queue(b);
}


bool effekt_bild(const struct sGIMP *welcher) {
  struct sBitmap *b = (struct sBitmap *) malloc(sizeof(struct sBitmap));
  memset(b, 0, sizeof(struct sBitmap));
  struct sCRGBA *bitmap = (struct sCRGBA *) calloc(LED_COUNT, sizeof(struct sCRGBA));
  memset(bitmap, 0, sizeof(struct sCRGBA) * LED_COUNT);
  gimp_rendern(bitmap, welcher, LED_COUNT_X / 2, LED_COUNT_Y / 2, REFPUNKT_MITTE, 192);
  b->bitmap = bitmap;
  uint32_t milliseconds = 5000;
  b->milliseconds = milliseconds;
  effekt_queue(b);
  return true;
}


bool effekt_smiley_grinsend() {
  return effekt_bild(&gimp_smiley_grinsend);
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
  "Grinse-Smiley",       true,   200, effekt_smiley_grinsend,
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


bool (*laufender_effekt)();


void effekte_setze_laufender_effekt(int welcher) {
    laufender_effekt = effekte[welcher].funktion;
}

// Siehe oben :-) C ist eigentlich noch cooler als Brainfuck.
bool (*wuerfele_effekt())() {
  int32_t x = random(effekte_summe_gewichte);
  struct sEffekt *e;
  for (int i = 0; i < EFFEKTE_ANZAHL && x >= 0; i++) {
    e = &effekte[i];
    if (!e->aktiv) continue;
    x -= e->gewichtung;
  }
  return e ? e->funktion : NULL;
}


void effekt_pipeline_fuellen() {
  //  if (!einaus) return;  // Wenn das Schild gerade nicht an ist, brauchen auch keine Effekte erzeugt zu werden.
  if (effekt_pipeline_laenge >= EFFEKTE_PIPELINE_MINDESTLAENGE) return;  // wir haben genug auf Halde.
  if (laufender_effekt == NULL) {
    if (!effekte_einaus) return;
    effekt_schedule_pause(random(effekt_pause_max - effekt_pause_min) + effekt_pause_min);
    laufender_effekt = wuerfele_effekt();
  }
  while (effekt_pipeline_laenge < EFFEKTE_PIPELINE_MAXIMALLAENGE && laufender_effekt != NULL) {
    bool abgeschlossen = (*laufender_effekt)(); // dann rufen wir die Funktion mal auf...
    if (abgeschlossen) {
      laufender_effekt = NULL;
    }
  }
}



// immer aufrufen, wenn sich der pipelineHead geändert hat, also wenn eine erste
// Konfiguration eingereiht wurde und vorher keine da war. Auch dann, gerade ein
// dequeue stattgefunden hat.
void effekt_scheduleDequeue(uint32_t milliseconds) {
  struct sTask *t = (struct sTask *) malloc(sizeof(struct sTask));
  memset(t, 0, sizeof(struct sTask));
  t->semaphore = &semaphore_naechstesEffektLayerAnzeigen;
  scheduleIn(t, milliseconds);
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
