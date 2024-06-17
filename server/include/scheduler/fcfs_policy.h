#ifndef FCFS_POLICY_H
#define FCFS_POLICY_H

#include "process.h"

typedef struct fcfs_queue *FCFSQueue;

FCFSQueue create_fcfs_queue();

void destroy_fcfs(FCFSQueue fcfs_queue);

int enqueue_fcfs(FCFSQueue fcfs_queue, Process process);

Process dequeue_fcfs(FCFSQueue fcfs_queue);

Process* status_fcfs(FCFSQueue fcfs_queue);

#endif
