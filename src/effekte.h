#ifndef __EFFEKTE_H
#define __EFFEKTE_H

#define PREF_NAMESPACE_EFFEKTE "effekte"

#include "effekt.h"

// jede braucht locker 3 kB RAM, also Vorsicht. Zu kurz kann zu Ruckeln führen, zu lang bringt nichts außer Speicherverbrauch.
#define EFFEKTE_PIPELINE_MINDESTLAENGE 2
#define EFFEKTE_PIPELINE_MAXIMALLAENGE 5

extern void effekte_pipeline_fuellen();

#endif