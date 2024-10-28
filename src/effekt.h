#ifndef __EFFEKT_H
#define __EFFEKT_H

#include <Arduino.h>
#include <Preferences.h>

extern uint16_t effekt_pipeline_laenge;
extern struct sBitmap *effekt_pipelineHead;
extern struct sBitmap *effekt_pipelineTail;


void setup_effekte();
void effekt_queue_bitmap(struct sBitmap *effekt);
void effekt_schedule_pause(uint32_t milliseconds);
void effekte_setze_laufender_effekt(int welcher); // sollte es echt nicht geben...
void effekt_pipeline_fuellen();
struct sBitmap *effekt_dequeue();



// Die brauchen wir für viele Effekte, und so müssen wir sie nur einmal definieren.
#define PREF_AKTIV "aktiv"
#define PREF_GEWICHTUNG "gewichtung"

#define PREF_AUSGEBEN(s, t, v) s += tag; s += "."; s += t; s += "="; s += v; s += "\n"


class Effekt {
  private:

  public:

    const char *tag;
    const char *name;
    const char *beschreibung;
    bool loeschbar;
    bool default_aktiv;
    bool aktiv;
    uint16_t default_gewichtung;
    uint16_t gewichtung;

    Effekt(bool loeschbar, bool aktiv, uint16_t gewichtung);
   ~Effekt();

    virtual bool doit();
    virtual void prefs_laden(Preferences& p);
    virtual void prefs_laden() final;
    virtual void prefs_schreiben(Preferences& p);
    virtual void prefs_schreiben() final;
    virtual void prefs_ausgeben(String& s);
    virtual void prefs_defaults();
};

#endif