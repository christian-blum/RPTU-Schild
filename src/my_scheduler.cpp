#include <Arduino.h>
#include "my_scheduler.h"


volatile struct sTask *taskQueue;
hw_timer_t *scheduler_timer;


void ARDUINO_ISR_ATTR scheduler_fireAndDiscard(struct sTask *task) {
  if (task->semaphore != NULL) *(task->semaphore) = true;
  if (task->function != NULL) (*(task->function))();
  free(task);
}

void scheduler_setAlarm(struct sTask *task) {
  uint64_t when = task->tick;
  uint64_t count = timerRead(scheduler_timer);
  if (count >= when) when = count + 100;
  else if (count >= when + 100) when += 100;
  // das geht nur in Arduino IDE     timerAlarm(scheduler_timer, when, false, 0);
  timerAlarmWrite(scheduler_timer, when, false);
  timerAlarmEnable(scheduler_timer);
}

void ARDUINO_ISR_ATTR scheduler_onTimer() {
  while (taskQueue != NULL) {
    struct sTask *q = (struct sTask *) taskQueue;
    uint64_t count = timerRead(scheduler_timer);
    if (count >= q->tick) {
      taskQueue = q->next;
      scheduler_fireAndDiscard(q); // will also free the memory
    }
    else break;
  }
  if (taskQueue != NULL) {
    scheduler_setAlarm((struct sTask *)taskQueue);
  }
}

void scheduler_start() {
  scheduler_timer = timerBegin(0, 80, true);
  if (scheduler_timer == NULL) {
    Serial.println("ARGHH!!!!! timer is NULL");
  }
  timerAttachInterrupt(scheduler_timer, &scheduler_onTimer, true);
  timerStart(scheduler_timer);
}

void schedule(struct sTask *task) {
  struct sTask *q;
  ATOMIC()
  { // lässt keine Interrupts zu, ohne dabei den vorherigen Zustand zu zerstören
    q = (struct sTask *) taskQueue;
    if (q == NULL) {
      task->next = NULL;
      taskQueue = task;
    }
    else {
      struct sTask *p = NULL;
      while (q->tick <= task->tick) {
        p = q;
        q = p->next;
        if (q == NULL) break;
      }
      if (p != NULL) {
        task->next = q;
        p->next = task;
      }
      else {
        task->next = q;
        taskQueue = task;
      }
    }
    // wenn wir jetzt der Kopf der Schlange sind, bestimmen wir, wann der Timer zum nächsten Mal feuert.
    if (taskQueue == task) {
      scheduler_setAlarm(task);
    }
  }
}

void scheduleIn(struct sTask *task, uint32_t milliseconds) {
  uint64_t timerTick = timerRead(scheduler_timer);
  task->tick = timerTick + milliseconds * 1000;
  schedule(task);
}

bool unschedule(struct sTask *task) {
  bool removed = false;
  struct sTask *q;
  ATOMIC() {
    q = (struct sTask *) taskQueue;
    if (q != NULL) {
      struct sTask *p;
      while (q != task) {
        p = q;
        q = p->next;
        if (q == NULL) break;
      }
      if (q == task) {
        p->next = q->next;
        removed = true;
      }
    }
    if (taskQueue) {
      scheduler_setAlarm((struct sTask *)taskQueue);
    }
  }
  return removed;
}

void reschedule(struct sTask *task) {
  unschedule(task);
  schedule(task);
};

void rescheduleIn(struct sTask *task, uint32_t milliseconds) {
  unschedule(task);
  uint64_t timerTick = timerRead(scheduler_timer);
  task->tick = timerTick + milliseconds * 1000;
  schedule(task);
};
