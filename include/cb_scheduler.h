#ifndef __MY_SCHEDULER_H
#define __MY_SCHEDULER_H

#include <Arduino.h>
#include <vector>

// Bei einer PLL-Frequenz von 80 MHz und einem Prescaler, der minimal 0 und maximal 255 sein kann
// (geteilt wird durch den Wert +1), liegt der Frequenzbereich zwischen 312,5 kHz und 80 MHz.
// Angabe in Hz. (Tatsächlich geht 40 MHz, aber nicht höher - wtf!) Wir verwenden Faktor 80.
#define SCHEDULER_PRESCALER 80
#define SCHEDULER_TIMER_FREQUENZ (80000000UL / SCHEDULER_PRESCALER)  // damit ist die Zeitbasis eine Mikrosekunde



typedef int cb_scheduler_handle_t;

class CB_Scheduler {
private:
  cb_scheduler_handle_t handleCounter;

  struct sSE {
    uint64_t when;
    uint64_t repeatMicros;
    void (*callMe)();
    volatile bool *setMe;
    int handle;
  };

  std::vector<sSE*> e;

  size_t divisionSearch(uint64_t x);
  void scheduleEvent(sSE *event);
  void scheduleEventMicros(sSE *s, uint64_t us);
  cb_scheduler_handle_t makeHandle();
  sSE *makeEventStruct();

public:
  CB_Scheduler();

  hw_timer_t *timer;

  bool begin();
  void end();
  cb_scheduler_handle_t callMeInSeconds(void (*function)(), uint32_t s, bool repeat = false);
  cb_scheduler_handle_t callMeInMilliseconds(void (*function)(), uint32_t ms, bool repeat = false);
  cb_scheduler_handle_t callMeInMicroseconds(void (*function)(), uint64_t us, bool repeat = false);
  cb_scheduler_handle_t setMeInSeconds(volatile bool *flag, uint32_t s, bool repeat = false);
  cb_scheduler_handle_t setMeInMilliseconds(volatile bool *flag, uint32_t ms, bool repeat = false);
  cb_scheduler_handle_t setMeInMicroseconds(volatile bool *flag, uint64_t us, bool repeat = false);
  bool rescheduleInSeconds(cb_scheduler_handle_t handle, uint32_t s);
  bool rescheduleInMilliseconds(cb_scheduler_handle_t handle, uint32_t ms);
  bool rescheduleInMicroseconds(cb_scheduler_handle_t handle, uint32_t us);
  bool cancel(cb_scheduler_handle_t handle);
  void loop();
};

#endif