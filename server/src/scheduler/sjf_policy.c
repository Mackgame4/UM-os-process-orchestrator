#include "scheduler/sjf_policy.h"
#include "stdio.h"
#include "stdlib.h"

typedef struct sjf_queue
{
  Process *arr;
  int size;
  int capacity;
  int next_id;
} *SJFQueue;

void bubble_down(SJFQueue sjf_queue, int index);
void bubble_up(SJFQueue sjf_queue, int index);

SJFQueue create_sjf_queue()
{
  SJFQueue queue = malloc(sizeof(struct sjf_queue));

  queue->size = 0;
  queue->capacity = 16;
  queue->next_id = 0;

  queue->arr = malloc(sizeof(Process *) * (long unsigned int)queue->capacity);

  printf("[DEBUG] - Created SJF queue with %d elements capacity;\n", queue->capacity);

  return queue;
}

void destroy_sjf(SJFQueue sjf_queue)
{
  free(sjf_queue->arr);
  free(sjf_queue);

  printf("[DEBUG] - Destroyed SJF queue;\n");
}

int enqueue_sjf(SJFQueue sjf_queue, Process process)
{
  if (sjf_queue->size == sjf_queue->capacity)
  {

    sjf_queue->capacity *= 2;
    sjf_queue->arr = realloc(sjf_queue->arr, sizeof(Process) * (long unsigned int)sjf_queue->capacity);

    printf("[DEBUG] - Resized SJF queue to %d elements;\n", sjf_queue->capacity);
  }

  int element_id = sjf_queue->next_id;

  sjf_queue->next_id++;
  sjf_queue->arr[sjf_queue->size] = process;
  sjf_queue->size++;

  bubble_up(sjf_queue, sjf_queue->size - 1);

  printf("[DEBUG] - Enqueued process %d to SJF queue;\n", element_id);

  return element_id;
}

Process dequeue_sjf(SJFQueue sjf_queue)
{
  if (sjf_queue->size == 0)
  {
    return NULL;
  }

  sjf_queue->size--;

  Process process = sjf_queue->arr[0];
  sjf_queue->arr[0] = sjf_queue->arr[sjf_queue->size];

  if (sjf_queue->size != 0)
  {
    bubble_down(sjf_queue, 0);
  }

  printf("[DEBUG] - Dequeued process from SJF queue - remain %d elements;\n", sjf_queue->size);

  return process;
}

Process* status_sjf(SJFQueue sjf_queue) {
  Process* status = malloc(sizeof(Process) * (long unsigned int)(sjf_queue->size + 1));

  for (int i = 0; i < sjf_queue->size; i++)
  {
    status[i] = sjf_queue->arr[i];
  }

  status[sjf_queue->size] = NULL;

  return status;
}

void bubble_down(SJFQueue sjf_queue, int index)
{
  int swap_index = -1;

  int l_child_index = (index * 2) + 1;
  int r_child_index = l_child_index + 1;

  int parent_time = ((Process)sjf_queue->arr[index])->estimated_runtime;

  if (l_child_index < sjf_queue->size)
  {
    int l_child_time = ((Process)sjf_queue->arr[l_child_index])->estimated_runtime;

    swap_index = parent_time > l_child_time ? l_child_index : swap_index;

    if (r_child_index < sjf_queue->size)
    {
      int r_child_time = ((Process)sjf_queue->arr[r_child_index])->estimated_runtime;

      swap_index = r_child_time > parent_time    ? swap_index
                   : r_child_time < l_child_time ? r_child_index
                                                 : l_child_index;
    }
  }

  if (swap_index == -1)
    return;

  Process swap = sjf_queue->arr[index];
  sjf_queue->arr[index] = sjf_queue->arr[swap_index];
  sjf_queue->arr[swap_index] = swap;

  bubble_down(sjf_queue, swap_index);
}

void bubble_up(SJFQueue sjf_queue, int index)
{
  if (index == 0)
    return;

  int parent_index = ((index + 1) / 2) - 1;

  int child_time = ((Process)sjf_queue->arr[index])->estimated_runtime;
  int parent_time = ((Process)sjf_queue->arr[parent_index])->estimated_runtime;

  if (parent_time > child_time)
  {
    Process swap = sjf_queue->arr[index];
    sjf_queue->arr[index] = sjf_queue->arr[parent_index];
    sjf_queue->arr[parent_index] = swap;

    bubble_up(sjf_queue, parent_index);
  }
}
