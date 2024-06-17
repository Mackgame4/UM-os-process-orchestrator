#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <signal.h>

#include "../../datapipe/globals.h"
#include "fifo/create.h"
#include "scheduler/scheduler.h"
#include "commands/command_interpreter.h"

int active_tasks = 0;                // Active tasks counter
Process* processes; // Array to store the processes in execution
int max_simultaneous = 0 ;                // Maximum number of simultaneous tasks
int process_count = 0;               // Number of processes (fifos) in execution
char task_path[MAX_FIFO_NAME];      // Path to the tasks directory

void execute_process(Process process)
{
    printf("[DEBUG] - Executing process %d: %s\n", process->id, process->command);
    int pid;
    pid = fork();
    if (pid == -1)
    {
        perror("Failed to fork");
        return;
    }
    if (pid == 0)
    { // Child process
        if (is_a_simple_process(process->command))
        {
            execute_simple_process(process, task_path);
        }
        else
        {
            execute_pipeline_process(process,task_path);
        }
        _exit(EXIT_SUCCESS);
    }
    else
    {
        process->process_id = pid;
        if (active_tasks < max_simultaneous)
        {
            processes[active_tasks++] = process;
        }
    }
}

void remove_processes(int index) {
    if (index < 0 || index >= active_tasks) {
        fprintf(stderr, "Index out of bounds\n");
        return;
    }

    // Moving all the elements to the left
    for (int i = index; i < active_tasks - 1; i++) {
        processes[i] = processes[i + 1];
    }

    // Decrementing the number of active tasks
    active_tasks--;
}

void handle_finished_task()
{
    int status;
    pid_t pid;
    while ((pid = waitpid(-1, &status, WNOHANG)) > 0)
    {
        for (int i = 0; i < active_tasks; i++)
        {
            if (processes[i]->process_id == pid)
            {
                // calculate duration of the process in seconds
                struct timeval end_time;
                gettimeofday(&end_time, NULL);
                // getting duration in seconds 
                double duration = (end_time.tv_sec - processes[i]->start_time.tv_sec) + (end_time.tv_usec - processes[i]->start_time.tv_usec) / 1000000.0;
               
                char *buffer = malloc(sizeof(char) * MAX_BUF_SIZE);

                char *command = get_command(processes[i]->command);

              sprintf(buffer, "%d %s %.4f seconds\n", processes[i]->id, command, duration);
              
                free(command);
                // writing to log file
                int fd = open(LOG_PATH, O_WRONLY | O_CREAT | O_APPEND, 0666);
                if (fd == -1)
                {
                    perror("Failed to open log file");
                    exit(EXIT_FAILURE);
                }
                int check = write(fd, buffer, strlen(buffer));
                if (check == -1)
                {
                    perror("Failed to write to log file");
                    exit(EXIT_FAILURE);
                }

                printf("[DEBUG] - Process %d has finished. Duration: %.4fseconds. Command: %s\n", processes[i]->id, duration, processes[i]->command);

                close(fd);
                free(buffer);
                remove_processes(i);
                break;
            }
        }
    }
}

void signal_handler(int signum)
{
    if (signum == SIGINT || signum == SIGTERM)
    {
        for (int i = 0; i < active_tasks; i++)
        { // kill all the processes before exiting
            kill(processes[i]->process_id, SIGKILL);
        }
        delete_fifo(S_FIFO_PATH); // Delete the server FIFO
        exit(EXIT_SUCCESS);
    }
}

// server will receive as input the output directory of the commands, the number of max tasks that can be executed simultaneously and the scheduling policy

int main(int argc, char *argv[])
{

   if (argc != 4)
    {
        // print usage with argv[0] as the program name
        char * usage = malloc(sizeof(char) * 100);
        sprintf(usage, "Usage: %s <output directory> <max tasks> <scheduling policy>\n", argv[0]);
        if (write(STDOUT_FILENO, usage, strlen(usage)) == -1)
        {
            perror("Failed to write to STDOUT");
            free (usage);
            exit(EXIT_FAILURE);
        }
        free(usage);
        exit(EXIT_FAILURE);
    }

    max_simultaneous = atoi(argv[2]);
    if (max_simultaneous <= 0)
    {
        if (write(STDOUT_FILENO,"Invalid number of max tasks\n", 29) == -1)
        {
            perror("Failed to write to STDOUT");
            exit(EXIT_FAILURE);
        }
        exit(EXIT_FAILURE);
    }

    if (strcmp(argv[3], "FCFS") != 0 && strcmp(argv[3], "SJF") != 0)
    {
        if (write(STDOUT_FILENO,"Invalid scheduling policy\n", 27) == -1)
        {
            perror("Failed to write to STDOUT");
            exit(EXIT_FAILURE);
        }
        exit(EXIT_FAILURE);
    }
    // Initialize the array of processes
    processes = malloc(sizeof(Process) * max_simultaneous);
    // defining task path as the output directory
    strcpy(task_path, argv[1]); 


    signal(SIGINT, signal_handler);  // CTRL + C
    signal(SIGTERM, signal_handler); // exit
    SchedulePolicy policy = strcmp(argv[3], "FCFS") == 0 ? FCFS : SJF;
    Scheduler scheduler = create_scheduler(policy);
    char *buffer = malloc(sizeof(char) * MAX_BUF_SIZE);
    char *response = malloc(sizeof(char) * MAX_BUF_SIZE);

    create_fifo(S_FIFO_PATH); // Create the server FIFO

    int server_fd = open(S_FIFO_PATH, O_RDONLY);
    if (server_fd == -1)
    {
        perror("Error opening server FIFO");
        exit(EXIT_FAILURE);
    }

    while (1)
    {
        handle_finished_task(); // Verify if any task has finished
        memset(buffer, 0, MAX_BUF_SIZE);
        memset(response, 0, MAX_BUF_SIZE);
        if (read(server_fd, buffer, MAX_BUF_SIZE) > 0)
        {
            char **command_args = command_interpreter(buffer);
            char *client_id = command_args[0];
            char *client_fifo_path = malloc(sizeof(char) * MAX_FIFO_NAME);
            sprintf(client_fifo_path, "%s_%s", C_FIFO_PATH, client_id);

            if (strcmp(command_args[1], "status") == 0)
            {
                status_writer(client_fifo_path, processes, active_tasks,scheduler);
                free(command_args);
                free(client_fifo_path);
                memset(buffer, 0, MAX_BUF_SIZE);
                memset(response, 0, MAX_BUF_SIZE);
                continue;
            }

            // advance 1 blank space in the buffer
            else
            {
                char *command = strchr(buffer, ' ') + 1;
                char *new_command = strdup(command);
                int id = enqueue_process(scheduler, new_command, atoi(command_args[2])); // Enqueue the process

                // Send the response to the client
                int client_fifo_fd = open(client_fifo_path, O_WRONLY);
                if (client_fifo_fd == -1)
                {
                    perror("Error opening client FIFO");
                    exit(EXIT_FAILURE);
                }
                sprintf(response, "Task ID = %d\n", id);

                int check = write(client_fifo_fd, response, strlen(response));
                if (check == -1)
                {
                    perror("Failed to write to client FIFO");
                }
                close(client_fifo_fd);
            }

            free(command_args);
            free(client_fifo_path);
            memset(buffer, 0, MAX_BUF_SIZE);
            memset(response, 0, MAX_BUF_SIZE);
        }

        while (active_tasks < max_simultaneous)
        {
            Process process = dequeue_process(scheduler);
            if (process != NULL)
            {
                execute_process(process);
            }
            else
            {
                break; // There are no more processes to execute
            }
        }
    }

    destroy_scheduler(scheduler);
    close(server_fd);
    free(buffer);
    free(response);

    return 0;
}