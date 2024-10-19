#include "uebergang_jump.h"
#include "einstellungen.h"


// Ein sehr simpler Übergang, der nur aus einem Schritt besteht: er setzt die neue Konfiguration, und fertig.
bool uebergang_jump(struct sKonfiguration *alteKonfiguration, struct sKonfiguration *neueKonfiguration) {
  uebergang_queueKonfiguration(neueKonfiguration, konfiguration_pause);
  return true;
}

