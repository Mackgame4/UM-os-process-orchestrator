#ifndef SCHEDULER_H
#define SCHEDULER_H

#include "process.h"

typedef enum schedule_policy
{
  FCFS,
  SJF,
} SchedulePolicy;

typedef struct scheduler *Scheduler;

Scheduler create_scheduler(SchedulePolicy schedule_policy);

void destroy_scheduler(Scheduler scheduler);

int enqueue_process(Scheduler scheduler, char *command, int estimated_runtime);

Process dequeue_process(Scheduler scheduler);

Process* scheduler_status(Scheduler scheduler);

#endif
