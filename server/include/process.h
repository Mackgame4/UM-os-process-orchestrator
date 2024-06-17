#ifndef PROCESS_H
#define PROCESS_H

#include <sys/time.h>

typedef struct process
{
  int id;
  char *command;
  int estimated_runtime;
  struct timeval start_time;
  int process_id;
} *Process;

#endif