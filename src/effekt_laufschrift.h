#ifndef __EFFEKT_LAUFSCHRIFT
#define __EFFEKT_LAUFSCHRIFT

#include "effekt.h"
#include "led_matrix.h"
#include "text_5x7.h"

// warum +2? Eins damit man bis zu einem kompletten Zeichen schieben kann, und eins für die terminierende \0
#define LAUFSCHRIFT_TEXTFRAGMENT_GROESSE ((LED_COUNT_X + TEXT_5X7_SPALTEN_PRO_ZEICHEN - 1) / TEXT_5X7_SPALTEN_PRO_ZEICHEN + 2)


class Effekt_Laufschrift : public Effekt {
  private:

    char *text; // nullterminiert, mit genügend vielen Leerzeichen davor und dahinter (LTG - 2)
    uint16_t zeichenzahl;
    uint16_t count; // zählt bis 6x Zeichenzahl
    uint16_t count_ende;
    volatile bool semaphore;  // brauche ich die?
    char textfragment[LAUFSCHRIFT_TEXTFRAGMENT_GROESSE];

  public:
    Effekt_Laufschrift(bool loeschbar, bool aktiv, uint16_t gewichtung);
   ~Effekt_Laufschrift();
    Effekt_Laufschrift(bool loeschbar, bool aktiv, uint16_t gewichtung, const char *anzeigetext, int16_t ypos, uint16_t millis, struct sCRGBA *schriftfarbe, struct sCRGBA *hintergrundfarbe);

    // bitte nicht extern manipulieren!
    int16_t y;  // Position auf Display
    const char *anzeigetext;
    struct sCRGBA *schriftfarbe;
    struct sCRGBA *hintergrundfarbe;
    uint16_t millis; // pro Schritt nach links

    void neuer_text(const char *anzeigetext, int16_t ypos, uint16_t millis, struct sCRGBA *schriftfarbe, struct sCRGBA *hintergrundfarbe);
    bool doit() override;
};


#endif