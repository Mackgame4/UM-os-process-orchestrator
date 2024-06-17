#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include "fifo/create.h"

int create_fifo(char *fifo_path)
{
  // Create FIFO file
  unlink(fifo_path);
  if (mkfifo(fifo_path, 0600) == -1)
  {
    perror("mkfifo");
    return 1;
  }
  return 0;
}

int delete_fifo(char *fifo_path)
{
  // Delete FIFO file
  if (unlink(fifo_path) == -1)
  {
    perror("unlink");
    return 1;
  }
  remove(fifo_path);
  return 0;
}