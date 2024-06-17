#include "scheduler/scheduler.h"
#include "scheduler/fcfs_policy.h"
#include "scheduler/sjf_policy.h"
#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include "time.h"
#include "process.h"

typedef struct queue *Queue;
typedef int (*EnqueueFunction)(Queue, Process);
typedef Process (*DequeueFunction)(Queue);
typedef Process* (*StatusFunction)(Queue);
typedef void (*DestroyFunction)();

typedef struct scheduler
{
  SchedulePolicy policy;

  Queue queue;

  EnqueueFunction enqueue_fun;
  DequeueFunction dequeue_fun;

  StatusFunction status_fun;

  DestroyFunction destroy_fun;

} *Scheduler;

Scheduler create_scheduler(SchedulePolicy schedule_policy)
{
  Scheduler scheduler;
  scheduler = malloc(sizeof(struct scheduler));

  scheduler->policy = schedule_policy;

  if (scheduler->policy == FCFS)
  {
    scheduler->queue = (Queue)create_fcfs_queue();
    scheduler->destroy_fun = (DestroyFunction)destroy_fcfs;

    scheduler->enqueue_fun = (EnqueueFunction)enqueue_fcfs;
    scheduler->dequeue_fun = (DequeueFunction)dequeue_fcfs;
    scheduler->status_fun = (StatusFunction)status_fcfs;
  }
  else if (scheduler->policy == SJF)
  {
    scheduler->queue = (Queue)create_sjf_queue();
    scheduler->destroy_fun = (DestroyFunction)destroy_sjf;

    scheduler->enqueue_fun = (EnqueueFunction)enqueue_sjf;
    scheduler->dequeue_fun = (DequeueFunction)dequeue_sjf;
    scheduler->status_fun = (StatusFunction)status_sjf;
  }
  else
  {
    printf("[ERROR] - Invalid Schedule Policy;\n");
    exit(1);
  }

  printf("[DEBUG] - Scheduler created with policy: %s;\n", schedule_policy == FCFS ? "FCFS" : schedule_policy == SJF ? "SJF"
                                                                                                                   : "None");

  return scheduler;
}

void destroy_scheduler(Scheduler scheduler)
{
  printf("[DEBUG] - Destroying Scheduler;\n");
  Process process;
  while ((process = scheduler->dequeue_fun(scheduler->queue)) != NULL)
  {
    free(process->command);
    free(process);
  }

  scheduler->destroy_fun(scheduler->queue);
  free(scheduler);
  printf("[DEBUG] - Scheduler Destroyed;\n");
}

int enqueue_process(Scheduler scheduler, char *command, int estimated_runtime)
{
  Process new_process = malloc(sizeof(struct process));

  new_process->command = strdup(command);
  new_process->estimated_runtime = estimated_runtime;
  gettimeofday(&new_process->start_time, NULL);

  new_process->id = scheduler->enqueue_fun(scheduler->queue, new_process);

  return new_process->id;
}

Process dequeue_process(Scheduler scheduler)
{
  return scheduler->dequeue_fun(scheduler->queue);
}

Process* scheduler_status(Scheduler scheduler) {
  return scheduler->status_fun(scheduler->queue);
}
