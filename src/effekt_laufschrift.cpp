#include "effekt_laufschrift.h"


static inline uint32_t rotateLeft(uint32_t x, int n) {
  return (x << n) | (x >> (32 - n)) & ~((-1 >> n) << n);
}



Effekt_Laufschrift::Effekt_Laufschrift(bool loeschbar, bool aktiv, uint16_t gewichtung, const char *tag, const char *name, const char *anzeigetext, int16_t ypos, uint16_t millis, struct sCRGBA schriftfarbe, struct sCRGBA hintergrundfarbe) : Effekt(loeschbar, aktiv, gewichtung) {
  text = nullptr;
  zeichenzahl = 0;
  count_ende = 0;

  parameter.push_back(P_YPOS);
  parameter.push_back(P_MILLIS);
  parameter.push_back(P_SF);
  parameter.push_back(P_HF);
  parameter.push_back(P_TEXT);

  uint32_t x = 0;
  for (const char *c = anzeigetext; *c; c++) {
    x = rotateLeft(x, 1) ^ (const unsigned char) *c; 
  }

  char *_tag = new char[strlen(tag)+1];
  strcpy(_tag, tag);
  Effekt::tag = _tag;
  char *_name = new char[strlen(name)+1];
  strcpy(_name, name);
  Effekt::name = _name;
  const char *_beschreibung = "Zeigt eine Laufschrift an, die sich im vorgegebenen Intervall jeweils einen Pixel nach links bewegt. Deutsche Umlaute sind erlaubt, viele Akzente ebenfalls; andere werden wahrscheinlich nicht korrekt dargestellt.";
  char *__beschreibung = new char[strlen(_beschreibung)+1];
  strcpy(__beschreibung, _beschreibung);
  Effekt_Laufschrift::beschreibung = __beschreibung;

  Effekt_Laufschrift::millis = Effekt_Laufschrift::default_millis = millis;
  Effekt_Laufschrift::schriftfarbe = Effekt_Laufschrift::default_schriftfarbe = schriftfarbe;
  Effekt_Laufschrift::hintergrundfarbe = Effekt_Laufschrift::default_hintergrundfarbe = hintergrundfarbe;
  Effekt_Laufschrift::ypos = Effekt_Laufschrift::default_ypos = ypos;
  int l = strlen(anzeigetext)+1;
  char *dat = new char[l];
  strcpy(dat, anzeigetext);
  Effekt_Laufschrift::default_anzeigetext = dat;
  char *at = new char[l];
  strcpy(at, anzeigetext);
  Effekt_Laufschrift::anzeigetext = at;
  text_padding();
}

Effekt_Laufschrift::~Effekt_Laufschrift() {
  if (text) { delete[] text; text = nullptr; zeichenzahl = 0; count_ende = 0; }
  if (anzeigetext) { delete[] anzeigetext; anzeigetext = nullptr; }
  if (default_anzeigetext) { delete[] default_anzeigetext; default_anzeigetext = nullptr; }
}

void Effekt_Laufschrift::text_padding() {
  int anzeigetext_laenge = strlen(anzeigetext);
  zeichenzahl = anzeigetext_laenge + (LAUFSCHRIFT_TEXTFRAGMENT_GROESSE - 2) * 2;
  char *neuer_text = new char[zeichenzahl + 1];
  char *c = neuer_text;
  for (int i = 0; i < LAUFSCHRIFT_TEXTFRAGMENT_GROESSE - 2; i++) *c++ = ' ';
  for (int i = 0; i < anzeigetext_laenge; i++) {
    *c++ = iso88591(anzeigetext[i]);
  }
  for (int i = 0; i < LAUFSCHRIFT_TEXTFRAGMENT_GROESSE - 2; i++) *c++ = ' ';
  *c = '\0';
  if (text) delete[] text;
  text = neuer_text; // bitte so lassen, das ist narrensicher (zum Beispiel Aufruf mit Pointer auf alten Text auf dem Heap)
  count_ende = (zeichenzahl - (LAUFSCHRIFT_TEXTFRAGMENT_GROESSE - 2)) * TEXT_5X7_SPALTEN_PRO_ZEICHEN;
}

void Effekt_Laufschrift::neuer_text(const char *anzeigetext) {
  char *at = new char[strlen(anzeigetext)+1];
  memcpy(at, anzeigetext, strlen(anzeigetext)+1);
  if (Effekt_Laufschrift::anzeigetext) delete[] Effekt_Laufschrift::anzeigetext; // bitte Reihenfolge so lassen, das ist narrensicher (z.B. Aufruf mit Pointer auf alten Heap-Text)
  Effekt_Laufschrift::anzeigetext = at;
  text_padding();
}

void Effekt_Laufschrift::neuer_text(const char *anzeigetext, int16_t ypos, uint16_t millis, struct sCRGBA schriftfarbe, struct sCRGBA hintergrundfarbe) {
  neuer_text(anzeigetext);
  Effekt_Laufschrift::ypos = ypos;
  Effekt_Laufschrift::millis = millis;
  Effekt_Laufschrift::schriftfarbe = schriftfarbe;
  Effekt_Laufschrift::hintergrundfarbe = hintergrundfarbe;
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

void Effekt_Laufschrift::prefs_laden(Preferences& p) {
  Effekt::prefs_laden(p);
  if (p.isKey(PREF_MILLIS)) {
    millis = p.getUShort(PREF_MILLIS, millis);
  }
  if (p.isKey(PREF_SCHRIFTFARBE)) {
    struct sCRGBA schriftfarbe;
    schriftfarbe.x = p.getULong(PREF_SCHRIFTFARBE, schriftfarbe.x);
  }
  if (p.isKey(PREF_HINTERGRUNDFARBE)) {
    struct sCRGBA hintergrundfarbe;
    hintergrundfarbe.x = p.getULong(PREF_HINTERGRUNDFARBE, hintergrundfarbe.x);
  }
  if (p.isKey(PREF_ANZEIGETEXT)) {
    String t = p.getString(PREF_ANZEIGETEXT, anzeigetext);
    neuer_text(t.c_str());
  }
  if (p.isKey(PREF_YPOS)) {
    ypos = p.getShort(PREF_YPOS, ypos);
  }
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