#ifndef __EFFEKT_GIMP
#define __EFFEKT_GIMP

#include "effekt.h"
#include "gimp_artwork.h"

#define PREF_DAUER "dauer"

class Effekt_GIMP : public Effekt {

  private:
    const struct sGIMP *artwork;
    
    const struct sEffektParameter P_DAUER = { "dauer", "Anzeigedauer", EPT_USHORT, 5, "ms", (void *) &dauer };

  public:
    Effekt_GIMP(bool loeschbar, bool aktiv, uint16_t gewichtung, const struct sGIMP *artwork, uint16_t dauer);
   ~Effekt_GIMP();

    uint16_t default_dauer;
    uint16_t dauer;

    bool doit() override;
    void prefs_laden(Preferences& p) override;
    void prefs_schreiben(Preferences& p) override;
    void prefs_ausgeben(String& s) override;
    void prefs_defaults() override;

};

#endif