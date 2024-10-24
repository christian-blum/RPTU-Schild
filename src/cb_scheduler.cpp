#include <Arduino.h>
#include "cb_scheduler.h"

#undef USE_MALLOC


CB_Scheduler::CB_Scheduler() {

}

bool CB_Scheduler::begin() {
  timer = timerBegin(0, SCHEDULER_PRESCALER, true);
  if (!timer) {
    log_e("no timer available");
    return false;
  }
  timerStart(timer);
  return true;
}

void CB_Scheduler::end() {
  timerStop(timer);
}

void CB_Scheduler::scheduleEvent(sSE *event) {
  size_t pos = divisionSearch(event->when);
  e.insert(e.begin() + pos, event);
}

cb_scheduler_handle_t CB_Scheduler::makeHandle() {
  if (!handleCounter) handleCounter++;
  return handleCounter++;
}

CB_Scheduler::sSE *CB_Scheduler::makeEventStruct() {
#ifdef USE_MALLOC
  sSE *x = (sSE *)malloc(sizeof(sSE));
#else
  sSE *x = new sSE;
#endif
  memset(x, 0, sizeof(sSE));
  x->handle = makeHandle();
  return x;
}

cb_scheduler_handle_t CB_Scheduler::callMeInMicroseconds(void (*function)(), uint64_t us, bool repeat) {
  sSE* s = makeEventStruct();
  s->callMe = function;
  if (repeat) s->repeatMicros = us; else s->repeatMicros = 0;
  scheduleEventMicros(s, us);
  return s->handle;
}

cb_scheduler_handle_t CB_Scheduler::callMeInMilliseconds(void (*function)(), uint32_t ms, bool repeat) {
  return callMeInMicroseconds(function, 1000ULL * ms, repeat);
}

cb_scheduler_handle_t CB_Scheduler::callMeInSeconds(void (*function)(), uint32_t s, bool repeat) {
  return callMeInMicroseconds(function, 1000000ULL * s, repeat);
}

void CB_Scheduler::scheduleEventMicros(sSE *s, uint64_t us) {
  uint64_t timerTick = timerRead(timer);
  s->when = timerTick + us;
  scheduleEvent(s);
}

cb_scheduler_handle_t CB_Scheduler::setMeInMicroseconds(volatile bool *flag, uint64_t us, bool repeat) {
  sSE* s = makeEventStruct();
  s->setMe = flag;
  if (repeat) s->repeatMicros = us; else s->repeatMicros = 0;
  scheduleEventMicros(s, us);
  return s->handle;
}

cb_scheduler_handle_t CB_Scheduler::setMeInMilliseconds(volatile bool *flag, uint32_t ms, bool repeat) {
  return setMeInMicroseconds(flag, 1000ULL * ms, repeat);
}

cb_scheduler_handle_t CB_Scheduler::setMeInSeconds(volatile bool *flag, uint32_t s, bool repeat) {
  return setMeInMicroseconds(flag, 1000000ULL * s, repeat);
}

#define FOOLPROOF
#ifndef FOOLPROOF

// funktioniert nicht wirklich
size_t CB_Scheduler::divisionSearch(uint64_t x) {
  size_t s = e.size();
  if (s == 0) return 0;
  int l = 0;
  int h = s - 1;
  while (l < h) {
    size_t m = l + (h - l) >> 1;
    Serial.printf("l=%d m=%d h=%d\n", l, m, h);
    if ((*e[m]).when == x) return m;
    if ((*e[m]).when < x) l = m + 1;
    else h = m - 1;
  }
  return h;
}

#else

// das hier funktioniert verlässlich
size_t CB_Scheduler::divisionSearch(uint64_t x) {
  int m = 0;
  while (m < e.size() && (*e[m]).when<x) m++;
  return m;
}

#endif

// cancel is an expensive operation
bool CB_Scheduler::cancel(cb_scheduler_handle_t handle) {
  bool found = false;
  std::vector<sSE*>::iterator x = e.begin();
  while (x < e.end()) {
    if (handle == (*x)->handle) {
      e.erase(x);
#ifdef USE_MALLOC
      free(*x);
#else
      delete (*x);
#endif
      found = true;
      break;
    }
    else x++;
  }
  return found;
}

// reschedule is an expensive operation
bool CB_Scheduler::rescheduleInMicroseconds(cb_scheduler_handle_t handle, uint32_t us) {
  std::vector<sSE*>::iterator x = e.begin();
  while (x != e.end()) {
    sSE *ev = *x;
    if (ev->handle == handle) {
      e.erase(x);
      scheduleEventMicros(ev, us);
      return true;
    }
    else x++;
  }
  return false;
}

bool CB_Scheduler::rescheduleInMilliseconds(cb_scheduler_handle_t handle, uint32_t ms) {
  return rescheduleInMicroseconds(handle, 1000L * ms);
}

bool CB_Scheduler::rescheduleInSeconds(cb_scheduler_handle_t handle, uint32_t s) {
  return rescheduleInMicroseconds(handle, 1000000L * s);
}

void CB_Scheduler::loop() {
  if (!e.size()) return;
  uint64_t timerTick = timerRead(timer);
  bool fired = true;
  while (e.size() > 0 && fired) {
    fired = false;
    sSE *x = e[0];
    if ((int64_t) x->when - (int64_t) timerTick <= 0) {
      e.erase(e.begin());
      if (x->callMe) (*(x->callMe))();
      if (x->setMe) *(x->setMe) = true;
      if (x->repeatMicros) {
        x->when += x->repeatMicros;
        scheduleEvent(x);
      }
      else {
#ifdef USE_MALLOC
        free(x);
#else
        delete x;
#endif
      }
      fired = true;
    }
  }
}


