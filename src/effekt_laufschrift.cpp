#include "effekt_laufschrift.h"


static inline uint32_t rotateLeft(uint32_t x, int n) {
  return (x << n) | (x >> (32 - n)) & ~((-1 >> n) << n);
}


Effekt_Laufschrift::Effekt_Laufschrift(bool loeschbar, bool aktiv, uint16_t gewichtung) : Effekt(loeschbar, aktiv, gewichtung) {
  text = nullptr;
  zeichenzahl = 0;
  count_ende = 0;
}

Effekt_Laufschrift::Effekt_Laufschrift(bool loeschbar, bool aktiv, uint16_t gewichtung, const char *anzeigetext, int16_t ypos, uint16_t millis, struct sCRGBA schriftfarbe, struct sCRGBA hintergrundfarbe) : Effekt_Laufschrift(loeschbar, aktiv, gewichtung) {
  uint32_t x = 0;
  for (const char *c = anzeigetext; *c; c++) {
    x = rotateLeft(x, 1) ^ (const unsigned char) *c; 
  }
  char *_tag = new char[12];
  sprintf(_tag, "el_%8.8x", x);
  tag = _tag;
  char *_name = new char[22];
  sprintf(_name, "Laufschrift %8.8x", x);
  name = _name;
  beschreibung = "Zeigt sporadisch eine vom System oder vom Administrator konfigurierte Laufschrift an, die sich im vorgegebenen Intervall jeweils einen Pixel nach links bewegt. Umlaute Ä, Ö und Ü sowie ß sind erlaubt, andere werden wahrscheinlich nicht korrekt dargestellt.";

  Effekt_Laufschrift::default_millis = millis;
  Effekt_Laufschrift::default_schriftfarbe = schriftfarbe;
  Effekt_Laufschrift::default_hintergrundfarbe = hintergrundfarbe;
  Effekt_Laufschrift::default_ypos = ypos;
  int l = strlen(anzeigetext)+1;
  char *dat = new char[strlen(anzeigetext)+1];
  memcpy(dat, anzeigetext, l);
  Effekt_Laufschrift::default_anzeigetext = dat;
  char *at = new char[strlen(anzeigetext)+1];
  memcpy(at, default_anzeigetext, strlen(default_anzeigetext)+1);
  Effekt_Laufschrift::anzeigetext = at;
}

Effekt_Laufschrift::~Effekt_Laufschrift() {
  if (text) { delete[] text; text = nullptr; zeichenzahl = 0; count_ende = 0; }
  if (anzeigetext) { delete[] anzeigetext; anzeigetext = nullptr; }
  if (default_anzeigetext) { delete[] default_anzeigetext; default_anzeigetext = nullptr; }
}

void Effekt_Laufschrift::neuer_text(const char *anzeigetext, int16_t ypos, uint16_t millis, struct sCRGBA schriftfarbe, struct sCRGBA hintergrundfarbe) {
  char *at = new char[strlen(anzeigetext)+1];
  memcpy(at, anzeigetext, strlen(anzeigetext)+1);
  if (Effekt_Laufschrift::anzeigetext) delete[] Effekt_Laufschrift::anzeigetext; // bitte Reihenfolge so lassen, das ist narrensicher (z.B. Aufruf mit Pointer auf alten Heap-Text)
  Effekt_Laufschrift::anzeigetext = at;
  Effekt_Laufschrift::ypos = ypos;
  Effekt_Laufschrift::millis = millis;
  Effekt_Laufschrift::schriftfarbe = schriftfarbe;
  Effekt_Laufschrift::hintergrundfarbe = hintergrundfarbe;

  zeichenzahl = strlen(anzeigetext) + (LAUFSCHRIFT_TEXTFRAGMENT_GROESSE - 2) * 2;
  char *neuer_text = new char[zeichenzahl + 1];
  char *c = neuer_text;
  for (int i = 0; i < LAUFSCHRIFT_TEXTFRAGMENT_GROESSE - 2; i++) *c++ = ' ';
  memcpy(c, anzeigetext, zeichenzahl); c += zeichenzahl;
  for (int i = 0; i < LAUFSCHRIFT_TEXTFRAGMENT_GROESSE - 2; i++) *c++ = ' ';
  *c = '\0';
  if (text) delete[] text;
  text = neuer_text; // bitte so lassen, das ist narrensicher (zum Beispiel Aufruf mit Pointer auf alten Text auf dem Heap)
  count_ende = (zeichenzahl - (LAUFSCHRIFT_TEXTFRAGMENT_GROESSE - 2)) * TEXT_5X7_SPALTEN_PRO_ZEICHEN;
  Effekt::prefs_schreiben();
}

#if 0
bool Effekt_Laufschrift::doit() { return true; }
#else
bool Effekt_Laufschrift::doit() {
  if (count < count_ende) { // damit es bei Textänderungen nicht knallt, wenn der neue Text kürzer ist!
    // wenn wir ein neues Zeichen beginnen müssen, bauen wir den textfragment-Puffer neu auf
    if (count % TEXT_5X7_SPALTEN_PRO_ZEICHEN == 0) { // 6 weil 5x7 font mit 1 Spalte Abstand
      memcpy(textfragment, text+(count / TEXT_5X7_SPALTEN_PRO_ZEICHEN), LAUFSCHRIFT_TEXTFRAGMENT_GROESSE - 1);
      textfragment[LAUFSCHRIFT_TEXTFRAGMENT_GROESSE - 1] = '\0';
    }
  
    // ansonsten wird der Puffer einfach nur verschoben gerendert
    struct sPosition p;
    p.y = ypos;
    p.x = -(count % TEXT_5X7_SPALTEN_PRO_ZEICHEN);

    struct sBitmap *b = (struct sBitmap *) malloc(sizeof(struct sBitmap));
    memset(b, 0, sizeof(struct sBitmap));
    struct sCRGBA *bitmap = (struct sCRGBA *) calloc(LED_COUNT, sizeof(struct sCRGBA));
    memset(bitmap, 0, sizeof(struct sCRGBA) * LED_COUNT);
    text_rendern(bitmap, p, &schriftfarbe, &hintergrundfarbe, (const char *)textfragment);
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
#endif

void Effekt_Laufschrift::prefs_laden(Preferences& p) {
  Effekt::prefs_laden(p);
  uint16_t millis = p.getUShort(PREF_ANZEIGETEXT, millis);
  struct sCRGBA schriftfarbe;
  schriftfarbe.x = p.getULong(PREF_SCHRIFTFARBE, schriftfarbe.x);
  struct sCRGBA hintergrundfarbe;
  hintergrundfarbe.x = p.getULong(PREF_HINTERGRUNDFARBE, hintergrundfarbe.x);
  String t = p.getString(PREF_ANZEIGETEXT, anzeigetext);
  int16_t ypos = p.getShort(PREF_YPOS, ypos);
  neuer_text(t.c_str(), ypos, millis, schriftfarbe, hintergrundfarbe);
}

void Effekt_Laufschrift::prefs_schreiben(Preferences& p) {
  Effekt::prefs_schreiben(p);

  if (p.getUShort(PREF_MILLIS) != millis) p.putUShort(PREF_MILLIS, millis);
  if (p.getULong(PREF_SCHRIFTFARBE) != schriftfarbe.x) p.putULong(PREF_SCHRIFTFARBE, schriftfarbe.x);
  if (p.getULong(PREF_HINTERGRUNDFARBE) != hintergrundfarbe.x) p.putULong(PREF_HINTERGRUNDFARBE, hintergrundfarbe.x);
  if (p.getShort(PREF_YPOS) != ypos) p.putShort(PREF_YPOS, ypos);
  if (anzeigetext) {
    String t = p.getString(PREF_ANZEIGETEXT, anzeigetext);
    if (t.length() != strlen(anzeigetext) || (t.c_str(), anzeigetext)) p.putString(PREF_ANZEIGETEXT, anzeigetext);
  }
}

void Effekt_Laufschrift::prefs_ausgeben(String& s) {
  Effekt::prefs_ausgeben(s);
  PREF_AUSGEBEN(s, PREF_ANZEIGETEXT, anzeigetext);
  PREF_AUSGEBEN(s, PREF_YPOS, ypos);
  char buf[9];
  sprintf(buf, "%8.8x", schriftfarbe.x);
  PREF_AUSGEBEN(s, PREF_SCHRIFTFARBE, buf);
  sprintf(buf, "%8.8x", hintergrundfarbe.x);
  PREF_AUSGEBEN(s, PREF_HINTERGRUNDFARBE, buf);
}

void Effekt_Laufschrift::prefs_defaults() {
  Effekt::prefs_defaults();
  neuer_text(default_anzeigetext, default_ypos, default_millis, default_schriftfarbe, default_hintergrundfarbe);
}