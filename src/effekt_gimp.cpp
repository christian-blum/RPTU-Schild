#include "effekt_gimp.h"
#include "led_matrix.h"

static inline uint32_t rotateLeft(uint32_t x, int n) {
  return (x << n) | (x >> (32 - n)) & ~((-1 >> n) << n);
}

Effekt_GIMP::Effekt_GIMP(bool loeschbar, bool aktiv, uint16_t gewichtung, const struct sGIMP *artwork, uint16_t dauer) : Effekt(loeschbar, aktiv, gewichtung) {
// FIXME name. desc als Argumente übergeben und speichern
  Effekt_GIMP::artwork = artwork;
  Effekt_GIMP::dauer = Effekt_GIMP::default_dauer = dauer;
  uint32_t x = 0;
  uint32_t l = artwork->width * artwork->height;
  for (const unsigned char *c = (const unsigned char *)artwork->data; l > 0; l--) {
    register uint32_t p = 0;
    for (register uint8_t cc = artwork->bytes_per_pixel; cc > 0; c++, cc--) {
      p = rotateLeft(p, 8) ^ *c;
    } 
    x = rotateLeft(x, 1) ^ p;
  }
  char *buffer = new char[12];
  sprintf(buffer, "eg_%8.8x", x);
  tag = buffer;
  buffer = new char[strlen(artwork->comment)+1];
  strcpy(buffer, artwork->comment);
  name = buffer;
  String b;
  b += "Zeigt sporadisch die Grafik \"";
  b += artwork->comment;
  b += "\" an.";
  buffer = new char[b.length()+1];
  memcpy(buffer, b.c_str(), b.length());
  buffer[b.length()] = '\0';
  beschreibung = buffer;
}

Effekt_GIMP::~Effekt_GIMP() {
  if (tag) { delete[] tag; tag = nullptr; }
  if (beschreibung) { delete[] beschreibung; beschreibung = nullptr; }
}

bool Effekt_GIMP::doit() {
  struct sBitmap *b = (struct sBitmap *) malloc(sizeof(struct sBitmap));
  memset(b, 0, sizeof(struct sBitmap));
  struct sCRGBA *bitmap = (struct sCRGBA *) calloc(LED_COUNT, sizeof(struct sCRGBA));
  memset(bitmap, 0, sizeof(struct sCRGBA) * LED_COUNT);
  gimp_rendern(bitmap, artwork, LED_COUNT_X / 2, LED_COUNT_Y / 2, REFPUNKT_MITTE, 192);
  b->bitmap = bitmap;
  uint32_t milliseconds = 5000;
  b->milliseconds = milliseconds;
  effekt_queue_bitmap(b);
  return true;
}

void Effekt_GIMP::prefs_laden(Preferences& p) {
  Effekt::prefs_laden(p);
  dauer = p.getUShort(PREF_DAUER, dauer);
}

void Effekt_GIMP::prefs_schreiben(Preferences& p) {
  Effekt::prefs_schreiben(p);
  if (p.getUShort(PREF_DAUER) != dauer) p.putUShort(PREF_DAUER, dauer);
}

void Effekt_GIMP::prefs_ausgeben(String& s) {
  Effekt::prefs_ausgeben(s);
  PREF_AUSGEBEN(s, PREF_DAUER, dauer);
}

void Effekt_GIMP::prefs_defaults() {
  Effekt::prefs_defaults();
  dauer = default_dauer;
}
