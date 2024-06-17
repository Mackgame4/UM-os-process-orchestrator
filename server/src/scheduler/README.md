# Scheduler

The scheduler will store all processes ready to run on a queue and, when requested, choose one based on a Scheduling Policy.

## The code

## Scheduler API

Here is a simple example of how to use the scheduler:

```c
#include "scheduler/scheduler.h"

int main() {
  Scheduler scheduler = create_scheduler(FCFS);

  enqueue_process(scheduler, "ls /etc/ -la", 10);

  // List of all processes in the scheduler
  Process* status = scheduler_status(scheduler)

  Process p = dequeue_process(scheduler);

  /*
    p->id == 1
    p->command == "ls /etc/ -la"
    p->estimated_runtime == 10
  */

  free(p);

  destroy_scheduler(scheduler);

  return 0;
}
```

## How create new policies

All the code is encapsulated, so you just need to create a new policy file with the following functions:

```c
// replace policy with the policy name

// This is a structure that can store everything that you need
typedef struct policy_queue* PolicyQueue;

Queue create_policy_queue();

// free all the memory allocated by the queue
void destroy_fcfs(FCFSQueue fcfs_queue);

int enqueue_policy(Queue policy_queue, Process process);

Process dequeue_policy(Queue policy_queue);

// Return an array with all the processes in the queue
Process* status_policy(Queue policy_queue);
```

Then you just need add the policy to the `scheduler.c` file:

```c
// don't forget to add YOUR_POLICY to the SchedulePolicy enum

if (scheduler->policy == YOUR_POLICY) {
  scheduler->queue = (Queue)create_policy_queue();
  scheduler->destroy_fun = (DestroyFunction)destroy_policy;
    
  scheduler->enqueue_fun = (EnqueueFunction)enqueue_policy;
  scheduler->dequeue_fun = (DequeueFunction)dequeue_policy;
  scheduler->status_fun = (StatusFunction)status_policy;
}
```

## Policies

### FCFS (First Come First Served)

The first process to arrive will be the first to be executed.

This module use a circular queue to store the processes, reusing the unqueued positions. When the queue is full, it will be resized to double of its size, and move all processes, in the same order, to the start of the new queue.

Here is a visualization of multiple enqueue and dequeue operations:

```plaintext
===================================================================
[DEBUG] - Created FCFS queue with 16 elements capacity;
[DEBUG] - Orchestrator created with policy: FCFS;
  ENQUEUED: [ x  _  _  _  _  _  _  _  _  _  _  _  _  _  _  _ ]
  ENQUEUED: [ x  x  _  _  _  _  _  _  _  _  _  _  _  _  _  _ ]
  ENQUEUED: [ x  x  x  _  _  _  _  _  _  _  _  _  _  _  _  _ ]
  ENQUEUED: [ x  x  x  x  _  _  _  _  _  _  _  _  _  _  _  _ ]
  ENQUEUED: [ x  x  x  x  x  _  _  _  _  _  _  _  _  _  _  _ ]
  ENQUEUED: [ x  x  x  x  x  x  _  _  _  _  _  _  _  _  _  _ ]
  ENQUEUED: [ x  x  x  x  x  x  x  _  _  _  _  _  _  _  _  _ ]
  ENQUEUED: [ x  x  x  x  x  x  x  x  _  _  _  _  _  _  _  _ ]
  ENQUEUED: [ x  x  x  x  x  x  x  x  x  _  _  _  _  _  _  _ ]
  ENQUEUED: [ x  x  x  x  x  x  x  x  x  x  _  _  _  _  _  _ ]
  ENQUEUED: [ x  x  x  x  x  x  x  x  x  x  x  _  _  _  _  _ ]
  ENQUEUED: [ x  x  x  x  x  x  x  x  x  x  x  x  _  _  _  _ ]
  ENQUEUED: [ x  x  x  x  x  x  x  x  x  x  x  x  x  _  _  _ ]
  ENQUEUED: [ x  x  x  x  x  x  x  x  x  x  x  x  x  x  _  _ ]
   DEQUEUE: [ _  x  x  x  x  x  x  x  x  x  x  x  x  x  _  _ ]
   DEQUEUE: [ _  _  x  x  x  x  x  x  x  x  x  x  x  x  _  _ ]
   DEQUEUE: [ _  _  _  x  x  x  x  x  x  x  x  x  x  x  _  _ ]
   DEQUEUE: [ _  _  _  _  x  x  x  x  x  x  x  x  x  x  _  _ ]
  ENQUEUED: [ _  _  _  _  x  x  x  x  x  x  x  x  x  x  x  _ ]
  ENQUEUED: [ _  _  _  _  x  x  x  x  x  x  x  x  x  x  x  x ]
  ENQUEUED: [ x  _  _  _  x  x  x  x  x  x  x  x  x  x  x  x ]
  ENQUEUED: [ x  x  _  _  x  x  x  x  x  x  x  x  x  x  x  x ]
  ENQUEUED: [ x  x  x  _  x  x  x  x  x  x  x  x  x  x  x  x ]
   DEQUEUE: [ x  x  x  _  _  x  x  x  x  x  x  x  x  x  x  x ]
  ENQUEUED: [ x  x  x  x  _  x  x  x  x  x  x  x  x  x  x  x ]
  ENQUEUED: [ x  x  x  x  x  x  x  x  x  x  x  x  x  x  x  x ]
   RESIZED: [ x  x  x  x  x  x  x  x  x  x  x  x  x  x  x  x  _  _  _  _  _  _  _  _  _  _  _  _  _  _  _  _ ]
  ENQUEUED: [ x  x  x  x  x  x  x  x  x  x  x  x  x  x  x  x  x  _  _  _  _  _  _  _  _  _  _  _  _  _  _  _ ]
  ENQUEUED: [ x  x  x  x  x  x  x  x  x  x  x  x  x  x  x  x  x  x  _  _  _  _  _  _  _  _  _  _  _  _  _  _ ]
  ENQUEUED: [ x  x  x  x  x  x  x  x  x  x  x  x  x  x  x  x  x  x  x  _  _  _  _  _  _  _  _  _  _  _  _  _ ]
  ENQUEUED: [ x  x  x  x  x  x  x  x  x  x  x  x  x  x  x  x  x  x  x  x  _  _  _  _  _  _  _  _  _  _  _  _ ]
   DEQUEUE: [ _  x  x  x  x  x  x  x  x  x  x  x  x  x  x  x  x  x  x  x  _  _  _  _  _  _  _  _  _  _  _  _ ]
   DEQUEUE: [ _  _  x  x  x  x  x  x  x  x  x  x  x  x  x  x  x  x  x  x  _  _  _  _  _  _  _  _  _  _  _  _ ]
   DEQUEUE: [ _  _  _  x  x  x  x  x  x  x  x  x  x  x  x  x  x  x  x  x  _  _  _  _  _  _  _  _  _  _  _  _ ]
   DEQUEUE: [ _  _  _  _  x  x  x  x  x  x  x  x  x  x  x  x  x  x  x  x  _  _  _  _  _  _  _  _  _  _  _  _ ]
===================================================================
```

### SJF (Shortest Job First)

The process with the shortest estimated runtime will be the first to be executed.

This module use a min-heap to store the processes. Here is a visualization of multiple enqueue and dequeue operations:

```plaintext
===================================================================
[DEBUG] - Created SJF queue with 16 elements capacity;
[DEBUG] - Scheduler created with policy: SJF;
    ENQUEUED: [ 13                                                   ]
  BUBBLED UP: [ 13                                                   ]
    ENQUEUED: [ 13 ====== 12                                         ]
  BUBBLED UP: [ 12 ====== 13                                         ]
    ENQUEUED: [ 12 ====== 13 11                                      ]
  BUBBLED UP: [ 11 ====== 13 12                                      ]
    ENQUEUED: [ 11 ====== 13 12 ====== 10                            ]
  BUBBLED UP: [ 10 ====== 11 12 ====== 13                            ]
    ENQUEUED: [ 10 ====== 11 12 ====== 13 9                          ]
  BUBBLED UP: [ 9 ====== 10 12 ====== 13 11                          ]
    ENQUEUED: [ 9 ====== 10 12 ====== 13 11 8                        ]
  BUBBLED UP: [ 8 ====== 10 9 ====== 13 11 12                        ]
    ENQUEUED: [ 8 ====== 10 9 ====== 13 11 12 7                      ]
  BUBBLED UP: [ 7 ====== 10 8 ====== 13 11 12 9                      ]
    ENQUEUED: [ 7 ====== 10 8 ====== 13 11 12 9 ====== 6             ]
  BUBBLED UP: [ 6 ====== 7 8 ====== 10 11 12 9 ====== 13             ]
    ENQUEUED: [ 6 ====== 7 8 ====== 10 11 12 9 ====== 13 5           ]
  BUBBLED UP: [ 5 ====== 6 8 ====== 7 11 12 9 ====== 13 10           ]
    ENQUEUED: [ 5 ====== 6 8 ====== 7 11 12 9 ====== 13 10 4         ]
  BUBBLED UP: [ 4 ====== 5 8 ====== 7 6 12 9 ====== 13 10 11         ]
    ENQUEUED: [ 4 ====== 5 8 ====== 7 6 12 9 ====== 13 10 11 3       ]
  BUBBLED UP: [ 3 ====== 4 8 ====== 7 5 12 9 ====== 13 10 11 6       ]
    ENQUEUED: [ 3 ====== 4 8 ====== 7 5 12 9 ====== 13 10 11 6 2     ]
  BUBBLED UP: [ 2 ====== 4 3 ====== 7 5 8 9 ====== 13 10 11 6 12     ]
    ENQUEUED: [ 2 ====== 4 3 ====== 7 5 8 9 ====== 13 10 11 6 12 1   ]
  BUBBLED UP: [ 1 ====== 4 2 ====== 7 5 3 9 ====== 13 10 11 6 12 8   ]
    ENQUEUED: [ 1 ====== 4 2 ====== 7 5 3 9 ====== 13 10 11 6 12 8 0 ]
  BUBBLED UP: [ 0 ====== 4 1 ====== 7 5 3 2 ====== 13 10 11 6 12 8 9 ]
    DEQUEUED: [ 9 ====== 4 1 ====== 7 5 3 2 ====== 13 10 11 6 12 8   ]
BUBBLED DOWN: [ 1 ====== 4 2 ====== 7 5 3 9 ====== 13 10 11 6 12 8   ]
    DEQUEUED: [ 8 ====== 4 2 ====== 7 5 3 9 ====== 13 10 11 6 12     ]
BUBBLED DOWN: [ 2 ====== 4 3 ====== 7 5 8 9 ====== 13 10 11 6 12     ]
    DEQUEUED: [ 12 ====== 4 3 ====== 7 5 8 9 ====== 13 10 11 6       ]
BUBBLED DOWN: [ 3 ====== 4 8 ====== 7 5 12 9 ====== 13 10 11 6       ]
    DEQUEUED: [ 6 ====== 4 8 ====== 7 5 12 9 ====== 13 10 11         ]
BUBBLED DOWN: [ 4 ====== 5 8 ====== 7 6 12 9 ====== 13 10 11         ]
    DEQUEUED: [ 11 ====== 5 8 ====== 7 6 12 9 ====== 13 10           ]
BUBBLED DOWN: [ 5 ====== 6 8 ====== 7 11 12 9 ====== 13 10           ]
    DEQUEUED: [ 10 ====== 6 8 ====== 7 11 12 9 ====== 13             ]
BUBBLED DOWN: [ 6 ====== 7 8 ====== 10 11 12 9 ====== 13             ]
    DEQUEUED: [ 13 ====== 7 8 ====== 10 11 12 9                      ]
BUBBLED DOWN: [ 7 ====== 10 8 ====== 13 11 12 9                      ]
    DEQUEUED: [ 9 ====== 10 8 ====== 13 11 12                        ]
BUBBLED DOWN: [ 8 ====== 10 9 ====== 13 11 12                        ]
    DEQUEUED: [ 12 ====== 10 9 ====== 13 11                          ]
BUBBLED DOWN: [ 9 ====== 10 12 ====== 13 11                          ]
    DEQUEUED: [ 11 ====== 10 12 ====== 13                            ]
BUBBLED DOWN: [ 10 ====== 11 12 ====== 13                            ]
    DEQUEUED: [ 13 ====== 11 12                                      ]
BUBBLED DOWN: [ 11 ====== 13 12                                      ]
    DEQUEUED: [ 12 ====== 13                                         ]
BUBBLED DOWN: [ 12 ====== 13                                         ]
    DEQUEUED: [ 13                                                   ]
BUBBLED DOWN: [ 13                                                   ]
BUBBLED DOWN: [                                                      ]
===================================================================
```