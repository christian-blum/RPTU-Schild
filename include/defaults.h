#ifndef __DEFAULTS_H
#define __DEFAULTS_H


#define SOFTWARE_RELEASE "1.1/20241111"
#define HAVE_WEBSERVER
#undef HAVE_BLUETOOTH
#undef HAVE_BACKDOOR
#define HAVE_FAVICON

#define TASTENWIEDERHOLZEIT 30 // und das sind 30 Millisekunden, also maximal 33 Inkrementschritte pro Sekunde
#define OSD_DAUER 3000

#define KONFIGURATION_PAUSE 5000  // Millisekunden


#define OSD_FARBE_HINTERGRUND 0xff000000 // ich finde, durchsichtig sieht nicht gut aus
#define OSD_FARBE_SCHRIFT 0xffc0c0c0
#define OSD_FARBE_RAHMEN 0xffc00000
#define OSD_FARBE_BALKEN 0xff00ffff
#define OSD_FARBE_BALKENHINTERGRUND OSD_FARBE_HINTERGRUND
#define OSD_FARBE_INAKTIV 0xff0000ff
#define OSD_FARBE_AKTIV 0xff00ff00

#define HELLIGKEIT_MIN 2
#define HELLIGKEIT_MAX 255
#define HELLIGKEIT_DEFAULT 128

#ifdef HAVE_BLUETOOTH
#define BT_DEFAULT_DEVICE_NAME "RPTU-Schild"
#endif


#define EFFEKT_PAUSE_MIN 120000
#define EFFEKT_PAUSE_MAX 360000

#ifdef HAVE_BLUETOOTH
#define BT_FARBE_HINTERGRUND 0xf0000000
#define BT_FARBE_EIN 0xf000ff00
#define BT_FARBE_AUS 0xff0000ff
#define BT_FARBE_DEL 0xff00ffff
#endif




// Dinge, die ich nicht vorhabe, konfigurierbar zu machen

#define HINTERGRUND_FAKTOR 8   // dunkler, wenn heller Vordergrund

#endif