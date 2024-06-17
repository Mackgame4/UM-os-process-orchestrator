#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>
#include <sys/stat.h>

#include "../../datapipe/globals.h"
#include "commands/command_sender.h"

int client_pid;

void signal_handler(int signum)
{
  if (signum == SIGINT || signum == SIGTERM)
  {
    send_delete_task_fifo(client_pid);
    exit(EXIT_SUCCESS);
  }
}

int main(int argc, char **argv)
{
  signal(SIGINT, signal_handler);  // CTRL + C
  signal(SIGTERM, signal_handler); // exit

  client_pid = getpid();
  // creating client FIFO

  create_client_fifo(client_pid);

  char *c_fifo = (char *)malloc(MAX_FIFO_NAME * sizeof(char));
  sprintf(c_fifo, "%s_%d", C_FIFO_PATH, client_pid);

  if (argc <= 1)
  {
    ask_for_command(client_pid);
  }
  else
  {
    // Command is argv except the first element
    char *command = (char *)malloc(MAX_COMMAND_LENGTH * sizeof(char));
    for (int i = 1; i < argc; i++)
    {
      strcat(command, argv[i]);
      if (i < argc - 1)
      {
        strcat(command, " ");
      }
    }
    send_command(client_pid, command);
    free(command);
  }

  // Read response from server

  int client_fd = open(c_fifo, O_RDONLY);
  if (client_fd == -1)
  {
    perror("Error opening client FIFO");
    exit(EXIT_FAILURE);
  }
  char *buffer = malloc(sizeof(char) * MAX_BUF_SIZE);
  ssize_t read_bytes;
  memset(buffer, 0, MAX_BUF_SIZE);
  // doing a while loop because one of the request can be a status request
  while ((read_bytes = read(client_fd, buffer, MAX_BUF_SIZE)) > 0)
  {
    buffer[read_bytes+1] = '\0';
    printf("%s", buffer);
    memset(buffer, 0, MAX_BUF_SIZE);
  }

  // Close and remove client FIFO , and deleting the FIFO and buffer
  close(client_fd);
  remove(c_fifo);
  free(c_fifo);
  free(buffer);

  return EXIT_SUCCESS;
}