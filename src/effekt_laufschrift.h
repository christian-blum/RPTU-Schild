#ifndef __EFFEKT_LAUFSCHRIFT
#define __EFFEKT_LAUFSCHRIFT

#include "effekt.h"
#include "led_matrix.h"
#include "text_5x7.h"

// warum +2? Eins damit man bis zu einem kompletten Zeichen schieben kann, und eins für die terminierende \0
#define LAUFSCHRIFT_TEXTFRAGMENT_GROESSE ((LED_COUNT_X + TEXT_5X7_SPALTEN_PRO_ZEICHEN - 1) / TEXT_5X7_SPALTEN_PRO_ZEICHEN + 2)

#define PREF_ANZEIGETEXT "anzeigetext"
#define PREF_SCHRIFTFARBE "schriftf"
#define PREF_HINTERGRUNDFARBE "hintergrundf"
#define PREF_MILLIS "millis"
#define PREF_YPOS "ypos"

class Effekt_Laufschrift : public Effekt {
  private:
    const struct sEffektParameter P_YPOS = { "ypos", "Zeile", EPT_SHORT, 5, nullptr, (void *) &ypos };
    const struct sEffektParameter P_MILLIS = { "millis", "Zeit pro Pixel", EPT_USHORT, 5, "ms", (void *) &millis };
    const struct sEffektParameter P_SF = { "sf", "Schriftfarbe", EPT_RGBA, 5, nullptr, (void *) &schriftfarbe };
    const struct sEffektParameter P_HF = { "hf", "Hintergrundfarbe", EPT_RGBA, 5, nullptr, (void *) &hintergrundfarbe };
    const struct sEffektParameter P_TEXT = { "text", "Text", EPT_TEXT, 30, nullptr, (void *) &anzeigetext };

    char *text; // nullterminiert, mit genügend vielen Leerzeichen davor und dahinter (LTG - 2)
    uint16_t zeichenzahl;
    uint16_t count; // zählt bis 6x Zeichenzahl
    uint16_t count_ende;
    volatile bool semaphore;  // brauche ich die?
    char textfragment[LAUFSCHRIFT_TEXTFRAGMENT_GROESSE];

    void text_padding();

  public:
   ~Effekt_Laufschrift();
    Effekt_Laufschrift(bool loeschbar, bool aktiv, uint16_t gewichtung, const char *anzeigetext, int16_t ypos, uint16_t millis, struct sCRGBA schriftfarbe, struct sCRGBA hintergrundfarbe);

    // bitte nichts davon extern manipulieren!
    const char *default_anzeigetext;
    const char *anzeigetext;
    int16_t default_ypos;
    int16_t ypos;  // Position auf Display
    struct sCRGBA default_schriftfarbe;
    struct sCRGBA schriftfarbe;
    struct sCRGBA default_hintergrundfarbe;
    struct sCRGBA hintergrundfarbe;
    uint16_t default_millis;
    uint16_t millis; // pro Schritt nach links

    void neuer_text(const char *anzeigetext, int16_t ypos, uint16_t millis, struct sCRGBA schriftfarbe, struct sCRGBA hintergrundfarbe);
    bool doit() override;
    void prefs_laden(Preferences& p) override;
    void prefs_schreiben(Preferences& p) override;
    void prefs_ausgeben(String& s) override;
    void prefs_defaults() override;

};


#endif