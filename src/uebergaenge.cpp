#include <Arduino.h>
#include <Preferences.h>

#include "led_matrix.h"
#include "uebergaenge.h"

#include "uebergang_jump.h"
#include "uebergang_blend.h"
#include "uebergang_morph.h"
#include "uebergang_slot_machine.h"
#include "uebergang_big_bang.h"
#include "uebergang_kringel.h"
#include "uebergang_epilleptischer_anfall.h"


// jede braucht locker 3 kB RAM, also Vorsicht. Zu kurz kann zu Ruckeln führen, zu lang bringt nichts außer Speicherverbrauch.
#define UEBERGAENGE_PIPELINE_MINDESTLAENGE 2
#define UEBERGAENGE_PIPELINE_MAXIMALLAENGE 5



struct sPosition morph_position(struct sPosition alt, struct sPosition neu, uint32_t morph_step, uint32_t morph_steps) {
  struct sPosition p;
  uint32_t w = morph_steps - morph_step;
  p.x = (w * alt.x + morph_step * neu.x) / morph_steps;
  p.y = (w * alt.y + morph_step * neu.y) / morph_steps;
  return p;
}

struct sCRGBA morph_color(struct sCRGBA alt, struct sCRGBA neu, uint32_t i, uint32_t morph_steps) {
  uint32_t w = morph_steps - i;
  struct sCRGBA c;
  c.r = (w * alt.r + i * neu.r) / morph_steps;
  c.g = (w * alt.g + i * neu.g) / morph_steps;
  c.b = (w * alt.b + i * neu.b) / morph_steps;
  c.alpha = (w * alt.alpha + i * neu.alpha) / morph_steps;
  return c;
}

static struct sKonfiguration letzte_konfiguration;  // Kopie (!!) der letzten Konfiguration - weil sie im Hintergrund auf dem Heap vernichtet wird.
static struct sKonfiguration aktuelle_zielkonfiguration;
static bool (*laufender_uebergang)(struct sKonfiguration *alt, struct sKonfiguration *neu);


struct sUebergangDef {
  const char *name;
  bool aktiv;
  uint16_t gewichtung;
  bool (*funktion)(struct sKonfiguration *alt, struct sKonfiguration *neu);
};

struct sUebergang {
  bool aktiv;
  uint16_t gewichtung;
  bool (*funktion)(struct sKonfiguration *alt, struct sKonfiguration *neu);
};


static const struct sUebergangDef uebergaenge_defaults[] = {
  "Jump",           true,  100, uebergang_jump,
  "Morph",          true,  100, uebergang_morph,
  "Blend",          true,  100, uebergang_blend,
  "Slot Machine",   true,  150, uebergang_slot_machine,
  "Big Bang",       true,  150, uebergang_bigbang,
  "Kringel links",  true,   75, uebergang_kringel_linksrum,
  "Kringel rechts", true,   75, uebergang_kringel_rechtsrum,
  "Epillepsie",     true,   20, uebergang_epilleptischer_anfall,
};

#define UEBERGAENGE_ANZAHL (sizeof(uebergaenge_defaults) / sizeof(uebergaenge_defaults[0]))

static struct sUebergang uebergaenge_alt[UEBERGAENGE_ANZAHL];
static uint32_t uebergaenge_summe_gewichte;

static void uebergaenge_gewichtungen_summieren() {
  uebergaenge_summe_gewichte = 0;
  for (int i = 0; i < UEBERGAENGE_ANZAHL; i++) {
    struct sUebergang *u;
    u = &uebergaenge_alt[i];
    if (u->aktiv) uebergaenge_summe_gewichte += u->gewichtung;
  }
}

#define UEBERGAENGE_DELIMITER ";"

#undef DEBUG_UEBERGAENGE_LADEN


void uebergaenge_prefs_laden(Preferences& p) {
  uebergang_morph_prefs_laden(p);
  uebergang_blend_prefs_laden(p);
  uebergang_bigbang_prefs_laden(p);
  uebergang_kringel_prefs_laden(p);
  uebergang_epilleptischer_anfall_prefs_laden(p);
  uebergang_slot_machine_prefs_laden(p);
}

void uebergaenge_prefs_schreiben(Preferences& p) {
  uebergang_morph_prefs_schreiben(p);
  uebergang_blend_prefs_schreiben(p);
  uebergang_bigbang_prefs_schreiben(p);
  uebergang_kringel_prefs_schreiben(p);
  uebergang_epilleptischer_anfall_prefs_schreiben(p);
  uebergang_slot_machine_prefs_schreiben(p);
}

void uebergaenge_prefs_ausgeben(String& s) {
  uebergang_morph_prefs_ausgeben(s);
  uebergang_blend_prefs_ausgeben(s);
  uebergang_bigbang_prefs_ausgeben(s);
  uebergang_kringel_prefs_ausgeben(s);
  uebergang_epilleptischer_anfall_prefs_ausgeben(s);
  uebergang_slot_machine_prefs_ausgeben(s);
}










void uebergaenge_laden() {
  Preferences p;
  p.begin("uebergaenge", true);
  for (int i = 0; i < UEBERGAENGE_ANZAHL; i++) {
    const struct sUebergangDef *d = &uebergaenge_defaults[i];
    struct sUebergang *u = &uebergaenge_alt[i];
    u->aktiv = d->gewichtung;
    u->gewichtung = d->gewichtung;
    u->funktion = d->funktion;
#ifdef DEBUG_UEBERGAENGE_LADEN
    Serial.print("Lade Übergang "); Serial.print(d->name); Serial.print(": ");
#endif
    if (p.isKey(d->name)) {
      String s = p.getString(d->name);
      char buf[s.length()+1];
      memcpy(buf, s.c_str(), s.length());
      buf[s.length()] = '\0';
#ifdef DEBUG_UEBERGAENGE_LADEN
      Serial.print(buf);
#endif
      char *v = strtok(buf, UEBERGAENGE_DELIMITER);
      if (v) {
        u->aktiv = (*v == 1);
        v = strtok(NULL, UEBERGAENGE_DELIMITER);
        if (v) {
          u->gewichtung = atoi(v);
        }
        else u->aktiv = false; // irgendwas war flasch
      }
      else u->aktiv = false;
    }
#ifdef DEBUG_UEBERGAENGE_LADEN
    Serial.print("   "); Serial.print(u->aktiv); Serial.print(" "); Serial.print(u->gewichtung);
    Serial.println();
#endif
  }
  p.end();
  uebergaenge_gewichtungen_summieren();
}

void uebergaenge_speichern() {
  Preferences p;
  p.begin("uebergaenge", false);
  for (int i = 0; i < UEBERGAENGE_ANZAHL; i++) {
    const struct sUebergangDef *d = &uebergaenge_defaults[i];
    const struct sUebergang *u = &uebergaenge_alt[i];
    String x = String(u->aktiv ? "1;":"0;");
    x += u->gewichtung;
    p.putString(d->name, x);
  }
  p.end();
}



void setup_uebergaenge() {
  uebergaenge_laden();
  letzte_konfiguration = konfiguration_wuerfeln();
}


// nicht erschrecken. :-) Wir definieren eine Funktion, die keine Parameter hat und als Rückgabewert einen Pointer auf eine Funktion mit zwei Argumenten hat. C ist fast so cool wie Brainfuck.
bool (*wuerfele_uebergang())(struct sKonfiguration *alt, struct sKonfiguration *neu) {
  if (uebergaenge_summe_gewichte == 0) return NULL; // da ist nichts zu wollen
  int32_t x = random(uebergaenge_summe_gewichte);
  struct sUebergang *u;
  for (int i = 0; i < UEBERGAENGE_ANZAHL && x >= 0; i++) {
    u = &uebergaenge_alt[i];
    if (!u->aktiv) continue;
    x -= u->gewichtung;
  }
  return u ? u->funktion : NULL;
}



void base_pipeline_fuellen() {
  //  if (!einaus) return;  // Wenn das Schild gerade nicht an ist, brauchen auch keine Konfigurationen erzeugt zu werden.
  if (base_pipeline_laenge >= UEBERGAENGE_PIPELINE_MINDESTLAENGE) return;  // wir haben genug auf Halde.
  if (laufender_uebergang == NULL) {
    laufender_uebergang = wuerfele_uebergang();
    aktuelle_zielkonfiguration = konfiguration_wuerfeln();
  }
  while (base_pipeline_laenge < UEBERGAENGE_PIPELINE_MAXIMALLAENGE && laufender_uebergang != NULL) {
    bool abgeschlossen = (*laufender_uebergang)(&letzte_konfiguration, &aktuelle_zielkonfiguration); // dann rufen wir die Funktion mal auf...
    if (abgeschlossen) {
      letzte_konfiguration = aktuelle_zielkonfiguration;
      laufender_uebergang = NULL;
    }
  }
}






/***************************************************************/
/* So, und ab hier machen wir es jetzt richtig, nach und nach. */
/***************************************************************/

/* Hier wird festgelegt, welche Übergänge es gibt. Die Parameter sind nur Defaults. */
Uebergang uebergaenge[] = {
  Uebergang_Jump(true, 100),
  Uebergang_Morph(true, 100, 40, 50),
  Uebergang_Blend(true, 100, 20, 50),
  Uebergang_Slot_Machine(true, 150),
  Uebergang_Big_Bang(true, 150, 40, 50),
  Uebergang_Kringel_linksrum(true, 75, 90, 40),
  Uebergang_Kringel_rechtsrum(true, 75, 90, 40),
  Uebergang_Epilleptischer_Anfall(true, 20, 30, 110),
};
