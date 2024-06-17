#include "scheduler/fcfs_policy.h"
#include "stdio.h"
#include "stdlib.h"

typedef struct fcfs_queue {
  Process* arr;
  int starting_index;
  int ending_index;
  int capacity;
  int next_id;
} *FCFSQueue;

int get_fcfs_queue_length(FCFSQueue fcfs_queue);

FCFSQueue create_fcfs_queue()
{
  FCFSQueue queue = malloc(sizeof(struct fcfs_queue));

  queue->next_id = 0;
  queue->capacity = 16;
  queue->starting_index = 0;
  queue->ending_index = -1;

  queue->arr = malloc(sizeof(Process*) * (long unsigned int)queue->capacity);

  printf("[DEBUG] - Created FCFS queue with %d processes capacity;\n", queue->capacity);

  return queue;
}

void destroy_fcfs(FCFSQueue fcfs_queue)
{
  free(fcfs_queue->arr);
  free(fcfs_queue);

  printf("[DEBUG] - Destroyed FCFS queue;\n");
}

int enqueue_fcfs(FCFSQueue fcfs_queue, Process process) {
  int queue_len = get_fcfs_queue_length(fcfs_queue);

  if (queue_len == fcfs_queue->capacity) {
    Process* new_queue = malloc(sizeof(Process) * (long unsigned int)fcfs_queue->capacity * 2);

    int new_queue_index = 0, old_queue_index = fcfs_queue->starting_index;
    do
    {
      new_queue[new_queue_index] = fcfs_queue->arr[old_queue_index];
      old_queue_index = (old_queue_index + 1) % fcfs_queue->capacity;
      new_queue_index++;
    } while (new_queue_index < fcfs_queue->capacity);

    free(fcfs_queue->arr);

    fcfs_queue->starting_index = 0;
    fcfs_queue->ending_index = fcfs_queue->capacity - 1;
    fcfs_queue->arr = new_queue;
    fcfs_queue->capacity *= 2;

    printf("[DEBUG] - Resized FCFS queue to %d processes;\n", fcfs_queue->capacity);
  }

  int process_id = fcfs_queue->next_id;

  fcfs_queue->next_id++;
  fcfs_queue->ending_index = (fcfs_queue->ending_index + 1) % fcfs_queue->capacity;
  fcfs_queue->arr[fcfs_queue->ending_index] = process;

  printf("[DEBUG] - Enqueued process %d to FCFS queue on position %d;\n", process_id, fcfs_queue->ending_index);

  return process_id;
}

Process dequeue_fcfs(FCFSQueue fcfs_queue) {
  if (fcfs_queue->ending_index == -1) {
    return NULL;
  }

  Process process = fcfs_queue->arr[fcfs_queue->starting_index];

  if (fcfs_queue->starting_index == fcfs_queue->ending_index)
  {
    fcfs_queue->starting_index = 0;
    fcfs_queue->ending_index = -1;
  }
  else
  {
    fcfs_queue->starting_index = (fcfs_queue->starting_index + 1) % fcfs_queue->capacity;
  }

  int queue_len = get_fcfs_queue_length(fcfs_queue);
  printf("[DEBUG] - Dequeued process from FCFS queue - remain %d processes;\n", queue_len);

  return process;
}

Process* status_fcfs(FCFSQueue fcfs_queue) {
  
  if (fcfs_queue->ending_index == -1 || get_fcfs_queue_length(fcfs_queue) == 0){
    return NULL;
  }
  Process* status = malloc(sizeof(Process) * (long unsigned int)(get_fcfs_queue_length(fcfs_queue) + 1));

  int status_index = 0, queue_index = fcfs_queue->starting_index;

do {
    status[status_index++] = fcfs_queue->arr[queue_index];
    queue_index = (queue_index + 1) % fcfs_queue->capacity;
} while (queue_index != (fcfs_queue->ending_index + 1) % fcfs_queue->capacity);

  status[status_index] = NULL;

  return status;
}

int get_fcfs_queue_length(FCFSQueue fcfs_queue)
{
  if (fcfs_queue->ending_index == -1)
    return 0;

  return fcfs_queue->starting_index <= fcfs_queue->ending_index
             ? fcfs_queue->ending_index - fcfs_queue->starting_index + 1
             : fcfs_queue->capacity - fcfs_queue->starting_index + fcfs_queue->ending_index + 1;
}
