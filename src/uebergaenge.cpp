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
static bool (*laufender_uebergang_alt)(struct sKonfiguration *alt, struct sKonfiguration *neu);


uint32_t uebergaenge_summe_gewichte;





/***************************************************************/
/* So, und ab hier machen wir es jetzt richtig, nach und nach. */
/***************************************************************/

/* Hier wird festgelegt, welche Übergänge es gibt. Die Parameter sind nur Defaults. */

Uebergang_Jump x_uebergang_jump(true, 100);
Uebergang_Morph x_uebergang_morph(true, 100, 40, 50);
Uebergang_Blend x_uebergang_blend(true, 100, 20, 50);
Uebergang_Slot_Machine x_uebergang_slot_machine(true, 150, 50, 0.2f, 6.0f, 0.02f, 0.03f);
Uebergang_Big_Bang x_uebergang_big_bang(true, 150, 40, 50);
Uebergang_Kringel_linksrum x_uebergang_kringel_linksrum(true, 75, 90, 40);
Uebergang_Kringel_rechtsrum x_uebergang_kringel_rechtsrum(true, 75, 90, 40);
Uebergang_Epilleptischer_Anfall x_uebergang_epilleptischer_anfall(true, 20, 30, 110);

std::array<Uebergang *, 8> uebergaenge = {
  &x_uebergang_jump,
  &x_uebergang_morph,
  &x_uebergang_blend,
  &x_uebergang_slot_machine,
  &x_uebergang_big_bang,
  &x_uebergang_kringel_linksrum,
  &x_uebergang_kringel_rechtsrum,
  &x_uebergang_epilleptischer_anfall,
};

Uebergang *laufender_uebergang;

static void __upl(Uebergang *u) {
    u->prefs_laden();
}

void uebergaenge_prefs_laden() {
  std::for_each(uebergaenge.begin(), uebergaenge.end(), __upl);
}

static void __ups(Uebergang *u) {
    u->prefs_schreiben();
}

void uebergaenge_prefs_schreiben() {
  std::for_each(uebergaenge.begin(), uebergaenge.end(), __ups);
}

void uebergaenge_prefs_ausgeben(String &s) {
  for (int i = 0; i < uebergaenge.size(); i++) {
    Uebergang *u = uebergaenge[i];
    u->prefs_ausgeben(s);
  }
}

Uebergang *wuerfele_uebergang() {
  if (uebergaenge_summe_gewichte == 0) return nullptr; // da ist nichts zu wollen
  int32_t x = random(uebergaenge_summe_gewichte);
  Uebergang *u = nullptr;
  for (int i = 0; i < uebergaenge.size() && x >= 0; i++) {
    u = uebergaenge[i];
    if (!u->aktiv) continue;
    x -= u->gewichtung;
  }
  return u;
}

void base_pipeline_fuellen() {
  //  if (!einaus) return;  // Wenn das Schild gerade nicht an ist, brauchen auch keine Konfigurationen erzeugt zu werden.
  if (base_pipeline_laenge >= UEBERGAENGE_PIPELINE_MINDESTLAENGE) return;  // wir haben genug auf Halde.
  if (!laufender_uebergang) {
    laufender_uebergang = wuerfele_uebergang();
    aktuelle_zielkonfiguration = konfiguration_wuerfeln();
  }
  while (laufender_uebergang != nullptr && base_pipeline_laenge < UEBERGAENGE_PIPELINE_MAXIMALLAENGE) {
    bool abgeschlossen = laufender_uebergang->doit(&letzte_konfiguration, &aktuelle_zielkonfiguration); // dann rufen wir die Funktion mal auf...
    if (abgeschlossen) {
      letzte_konfiguration = aktuelle_zielkonfiguration;
      laufender_uebergang = nullptr;
    }
  }
}

void uebergaenge_gewichtungen_summieren() {
  uebergaenge_summe_gewichte = 0;
  for (int i = 0; i < uebergaenge.size(); i++) {
    Uebergang *u;
    u = uebergaenge[i];
    if (u->aktiv) uebergaenge_summe_gewichte += u->gewichtung;
  }
}

void setup_uebergaenge() {
  uebergaenge_prefs_laden();
  uebergaenge_gewichtungen_summieren();
  letzte_konfiguration = konfiguration_wuerfeln();
}
