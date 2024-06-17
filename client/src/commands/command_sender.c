#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include "../../datapipe/globals.h"
#include "commands/command_sender.h"

#define TERMINAL_YELLOW "\033[0;33m"
#define TERMINAL_RESET "\033[0m"

void create_client_fifo(int client_pid)
{
    char *c_fifo = (char *)malloc(MAX_FIFO_NAME * sizeof(char));
    sprintf(c_fifo, "%s_%d", C_FIFO_PATH, client_pid);
    mkfifo(c_fifo, 0666);
    free(c_fifo);
    return;
}

void ask_text()
{
    const char *prompt = TERMINAL_YELLOW "Enter a command:\n >> " TERMINAL_RESET;
    if (write(STDOUT_FILENO, prompt, strlen(prompt)) == -1)
    {
        perror("write");
        exit(1);
    }
}

int ask_for_command(int client_pid)
{
    int s_fd = open(S_FIFO_PATH, O_WRONLY, 0600); // open the server FIFO
    if (s_fd == -1)
    {
        perror("open");
        return 1;
    }

    char *command = malloc(sizeof(char) * MAX_BUF_SIZE);

    ask_text();
    while (read(STDIN_FILENO, command, MAX_BUF_SIZE) > 0)
    {
        ask_text();
        char *buffer = malloc(sizeof(char) * MAX_BUF_SIZE);
        // buffer = <client_pid> <command>
        sprintf(buffer, "%d %s", client_pid, command);
        if (write(s_fd, buffer, strlen(buffer)) == -1)
        {
            perror("write");
            return 1;
        }
        memset(command, 0, MAX_BUF_SIZE);
        free(buffer);
    }

    close(s_fd);
    free(command);
    return 0;
}

int send_command(int client_pid, char *command)
{
    int s_fd = open(S_FIFO_PATH, O_WRONLY, 0600); // open the server FIFO
    if (s_fd == -1)
    {
        perror("open");
        return 1;
    }

    char *buffer = malloc(sizeof(char) * MAX_BUF_SIZE);
    // buffer = <client_pid> <command>
    sprintf(buffer, "%d %s", client_pid, command);
    if (write(s_fd, buffer, strlen(buffer)) == -1)
    { // write the command to the server FIFO
        perror("write");
        return 1;
    }
    memset(buffer, 0, MAX_BUF_SIZE);
    free(buffer);
    close(s_fd);
    return 0;
}

void send_create_task_fifo(int client_pid)
{
    char *create_command = (char *)malloc(MAX_COMMAND_LENGTH * sizeof(char));
    sprintf(create_command, "create_taskuser %s_%d", C_FIFO_PATH, client_pid);
    send_command(client_pid, create_command);
    free(create_command);
}

void send_delete_task_fifo(int client_pid)
{
    char *delete_command = (char *)malloc(MAX_COMMAND_LENGTH * sizeof(char));
    sprintf(delete_command, "delete_taskuser %s_%d", C_FIFO_PATH, client_pid);
    send_command(client_pid, delete_command);
    free(delete_command);
}