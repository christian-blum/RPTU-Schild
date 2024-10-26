#include "effekt_laufschrift.h"


Effekt_Laufschrift::Effekt_Laufschrift(bool loeschbar, bool aktiv, uint16_t gewichtung) : Effekt(loeschbar, aktiv, gewichtung) {
  text = nullptr;
  zeichenzahl = 0;
  count_ende = 0;
}

Effekt_Laufschrift::Effekt_Laufschrift(bool loeschbar, bool aktiv, uint16_t gewichtung, const char *anzeigetext, int16_t ypos, uint16_t millis, struct sCRGBA *schriftfarbe, struct sCRGBA *hintergrundfarbe) : Effekt_Laufschrift(loeschbar, aktiv, gewichtung) {
  neuer_text(anzeigetext, ypos, millis, schriftfarbe, hintergrundfarbe);
}


Effekt_Laufschrift::~Effekt_Laufschrift() {
  if (text) {
    delete[] text;
    text = nullptr;
  }
}

void Effekt_Laufschrift::neuer_text(const char *anzeigetext, int16_t ypos, uint16_t millis, struct sCRGBA *schriftfarbe, struct sCRGBA *hintergrundfarbe) {
  Effekt_Laufschrift::anzeigetext = anzeigetext;
  Effekt_Laufschrift::y = ypos;
  Effekt_Laufschrift::millis = millis;
  Effekt_Laufschrift::schriftfarbe = schriftfarbe;
  Effekt_Laufschrift::hintergrundfarbe = hintergrundfarbe;

  if (text) delete[] text;
  zeichenzahl = strlen(anzeigetext) + (LAUFSCHRIFT_TEXTFRAGMENT_GROESSE - 2) * 2;
  text = new char[zeichenzahl + 1];
  char *c = text;
  for (int i = 0; i < LAUFSCHRIFT_TEXTFRAGMENT_GROESSE - 2; i++) *c++ = ' ';
  memcpy(c, anzeigetext, zeichenzahl); c += zeichenzahl;
  for (int i = 0; i < LAUFSCHRIFT_TEXTFRAGMENT_GROESSE - 2; i++) *c++ = ' ';
  *c = '\0';
  count_ende = (zeichenzahl - (LAUFSCHRIFT_TEXTFRAGMENT_GROESSE - 2)) * TEXT_5X7_SPALTEN_PRO_ZEICHEN;
}


bool Effekt_Laufschrift::doit() {
  if (count < count_ende) { // damit es bei Textänderungen nicht knallt, wenn der neue Text kürzer ist!
    // wenn wir ein neues Zeichen beginnen müssen, bauen wir den textfragment-Puffer neu auf
    if (count % TEXT_5X7_SPALTEN_PRO_ZEICHEN == 0) { // 6 weil 5x7 font mit 1 Spalte Abstand
      memcpy(textfragment, text+(count / TEXT_5X7_SPALTEN_PRO_ZEICHEN), LAUFSCHRIFT_TEXTFRAGMENT_GROESSE - 1);
      textfragment[LAUFSCHRIFT_TEXTFRAGMENT_GROESSE - 1] = '\0';
    }
    // ansonsten wird der Puffer einfach nur verschoben gerendert
    struct sPosition p;
    p.y = y;
    p.x = -(count % TEXT_5X7_SPALTEN_PRO_ZEICHEN);

    struct sBitmap *b = (struct sBitmap *) malloc(sizeof(struct sBitmap));
    memset(b, 0, sizeof(struct sBitmap));
    struct sCRGBA *bitmap = (struct sCRGBA *) calloc(LED_COUNT, sizeof(struct sCRGBA));
    memset(bitmap, 0, sizeof(struct sCRGBA) * LED_COUNT);
    text_rendern(bitmap, p, schriftfarbe, hintergrundfarbe, (const char *)textfragment);
    b->bitmap = bitmap;
    b->milliseconds = millis;
    effekt_queue_bitmap(b);
    count++;
  }
  if (count >= count_ende) {
    count = 0;
    return true;
  }
  return false;
}
