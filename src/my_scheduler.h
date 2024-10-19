#ifndef __MY_SCHEDULER_H
#define __MY_SCHEDULER_H

#include <Arduino.h>

// Bei einer PLL-Frequenz von 80 MHz und einem Prescaler, der minimal 0 und maximal 255 sein kann
// (geteilt wird durch den Wert +1), liegt der Frequenzbereich zwischen 312,5 kHz und 80 MHz.
// Angabe in Hz. (Tatsächlich geht 40 MHz, aber nicht höher - wtf!)
#define SCHEDULER_TIMER_FREQUENZ 1000000  // damit ist die Zeitbasis eine Mikrosekunde


/***********************************************************
 *                                                         *
 * Scheduler                                               *
 *                                                         *
 ***********************************************************/

/* Geklaut von SimplyAtomic - das Einbinden hat nicht richtig funktioniert, aber es sind ja nur ein paar Zeilen, effektiv... */

static inline void SA_iRestore(const  uint32_t *__s)
{
    XTOS_RESTORE_INTLEVEL(*__s);
}

// Note value can be 0-15, 0 = Enable all interrupts, 15 = no interrupts
#define SA_ATOMIC_RESTORESTATE uint32_t _sa_saved              \
    __attribute__((__cleanup__(SA_iRestore))) = XTOS_DISABLE_LOWPRI_INTERRUPTS


/*************** MACRO **********************/
#define ATOMIC()                                               \
for ( SA_ATOMIC_RESTORESTATE, _sa_done =  1;                   \
    _sa_done; _sa_done = 0 )

/* danke schön! */

extern hw_timer_t *scheduler_timer;

 struct sTask {
  uint64_t tick;
  volatile bool * semaphore;
  void (*function)();
  struct sTask *next;
 };

void scheduler_start();
void scheduler_setAlarm(struct sTask *task);
void schedule(struct sTask *task);
void scheduleIn(struct sTask *task, uint32_t milliseconds);
bool unschedule(struct sTask *task);
void reschedule(struct sTask *task);
void rescheduleIn(struct sTask *task, uint32_t milliseconds);

#endif