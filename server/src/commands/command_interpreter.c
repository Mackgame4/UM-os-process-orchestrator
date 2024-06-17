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
#include "scheduler/scheduler.h"

#include "../../datapipe/globals.h"
#include "commands/command_interpreter.h"

char **command_interpreter(char *command)
{
    char **command_args = (char **)malloc(MAX_COMMAND_LENGTH * sizeof(char *));
    int i = 0;
    int in_quotes = 0;
    size_t arg_start = 0;
    size_t command_length = strlen(command);
    for (size_t j = 0; j <= command_length; j++)
    {
        if (command[j] == '"' && (j == 0 || command[j - 1] != '\\'))
        {
            in_quotes = !in_quotes;
        }
        else if ((command[j] == ' ' && !in_quotes) || command[j] == '\0')
        {
            size_t arg_length = j - arg_start;
            command_args[i] = (char *)malloc((arg_length + 1) * sizeof(char));
            strncpy(command_args[i], command + arg_start, arg_length);
            command_args[i][arg_length] = '\0'; // Null-terminate the string
            arg_start = j + 1;                  // Next argument starts after the space
            i++;
        }
    }

    // Remove the quotes from the arguments
    for (int j = 0; j < i; j++)
    {
        size_t arg_length = strlen(command_args[j]);
        if (command_args[j][0] == '"' && command_args[j][arg_length - 1] == '"')
        {
            command_args[j]++;
            command_args[j][arg_length - 2] = '\0';
        }
    }

    // Remove unwanted characters from the arguments (like "\n", "\r", etc.)
    for (int j = 0; j < i; j++)
    {
        size_t arg_length = strlen(command_args[j]);
        for (size_t k = 0; k < arg_length; k++)
        {
            if (command_args[j][k] == '\n' || command_args[j][k] == '\r')
            {
                command_args[j][k] = '\0';
            }
        }
    }

    // Free memory and set extra pointers to NULL
    while (i < MAX_COMMAND_LENGTH)
    {
        command_args[i] = NULL;
        i++;
    }

    return command_args;
}

char *get_command(char *command)
{

    char *buffer = strdup(command); // Creating a copy of the command
    if (buffer == NULL)
    {
        fprintf(stderr, "Memory allocation failed\n");
        return NULL;
    }

    char *token;
    char *result = malloc(sizeof(char) * MAX_BUF_SIZE);
    if (result == NULL)
    {
        fprintf(stderr, "Memory allocation failed\n");
        free(buffer);
        return NULL;
    }

    // execute
    token = strtok(buffer, " ");

    // time
    token = strtok(NULL, " ");

    // -u// -p
    token = strtok(NULL, " ");

    token = strtok(NULL, "");

    if (token)
    {
        strncpy(result, token, MAX_BUF_SIZE - 1); // Copies the command to the result string
        result[MAX_BUF_SIZE - 1] = '\0';          // Null-terminates the string
    }
    else
    {
        *result = '\0'; // Empty string
    }

    free(buffer); // Free the buffer
    return result;
}

char *get_command_pipeline(char *command)
{
    // Command is of type "execute 10 -p ls -l | grep '.txt' | wc -l"
    char *buffer = strdup(command); // Creating a copy of the command
    if (buffer == NULL)
    {
        fprintf(stderr, "Memory allocation failed\n");
        return NULL;
    }

    // Use strtok to tokenize the string
    const char *delim = " ";
    char *token;
    int foundP = 0;

    // Tokenize up to "-p"
    for (token = strtok(buffer, delim); token != NULL; token = strtok(NULL, delim))
    {
        if (strcmp(token, "-p") == 0)
        {
            foundP = 1;
            break;
        }
    }

    if (!foundP)
    {
        fprintf(stderr, "Command switch '-p' not found in the command string.\n");
        free(buffer);
        return NULL;
    }

    // The rest of the string after "-p" is the command
    token = strtok(NULL, ""); // Grab the rest of the string after "-p"

    if (token == NULL)
    {
        fprintf(stderr, "No command found after '-p'\n");
        free(buffer);
        return NULL;
    }

    // Duplicate the result to return, because we will free the buffer
    char *result = strdup(token);
    if (result == NULL)
    {
        fprintf(stderr, "Memory allocation failed for result\n");
    }

    // Clean up the original buffer
    free(buffer);
    return result;
}

void status_writer(char *client_fifo_path, Process processes[], int active_tasks, Scheduler scheduler)
{
    if (fork() == 0)
    {



        char *buffer = malloc(sizeof(char) * MAX_BUF_SIZE);
        if (buffer == NULL)
        {
            perror("Failed to allocate memory for buffer");
            exit(EXIT_FAILURE);
        }

        int client_fifo_fd = open(client_fifo_path, O_WRONLY);

        if (client_fifo_fd == -1)
        {
            perror("Error opening client FIFO");
            free(buffer);
            exit(EXIT_FAILURE);
        }

        // writing the header of the Scheduled tasks
        snprintf(buffer, MAX_BUF_SIZE, "Scheduled\n");
        
        if ( write(client_fifo_fd, buffer, strlen(buffer)) == -1)
        {
            perror("Failed to write to client FIFO");
            free(buffer);
            close(client_fifo_fd);
            exit(EXIT_FAILURE);
        }
        memset(buffer, 0, MAX_BUF_SIZE);

        // getting the queue elements of the scheduler

        Process *queue = scheduler_status(scheduler);
        int index = 0;
        while ( queue != NULL && queue[index] != NULL)
        {
            char *command = get_command(queue[index]->command);
            if (command == NULL)
            {
                perror("Failed to get command");
                memset(buffer, 0, MAX_BUF_SIZE);
                continue;
            }
            snprintf(buffer, MAX_BUF_SIZE, "Task ID = %d %s\n", queue[index]->id, command);
            int check = write(client_fifo_fd, buffer, strlen(buffer));
            if (check == -1)
            {
                perror("Failed to write to client FIFO");
                free(command);
                memset(buffer, 0, MAX_BUF_SIZE);
                continue;
            }
            memset(buffer, 0, MAX_BUF_SIZE);
            free(command);
            index++;
        }

        memset(buffer, 0, MAX_BUF_SIZE);
        if (queue != NULL)
        {
            free(queue);
        }

        // printing a line full of ----- to separate the scheduled tasks from the executed tasks
        snprintf(buffer, MAX_BUF_SIZE, "----------------------------------------\n");
        if (write(client_fifo_fd, buffer, strlen(buffer)) == -1)
        {
            perror("Failed to write to client FIFO");
            free(buffer);
            close(client_fifo_fd);
            exit(EXIT_FAILURE);
        }
        memset(buffer, 0, MAX_BUF_SIZE);


        // Send the header
        snprintf(buffer, MAX_BUF_SIZE, "Executing\n");
        int check = write(client_fifo_fd, buffer, strlen(buffer));
        if (check == -1)
        {
            perror("Failed to write to client FIFO");
            free(buffer);
            close(client_fifo_fd);
            exit(EXIT_FAILURE);
        }
        memset(buffer, 0, MAX_BUF_SIZE);

        // Send the processes in execution
        for (int i = 0; i < active_tasks; i++)
        {
            char *command = get_command(processes[i]->command);
            if (command == NULL)
            {
                perror("Failed to get command");
                memset(buffer, 0, MAX_BUF_SIZE);
                continue;
            }
            snprintf(buffer, MAX_BUF_SIZE, "Task ID = %d %s\n", processes[i]->id, command);
            check = write(client_fifo_fd, buffer, strlen(buffer));
            if (check == -1)
            {
                perror("Failed to write to client FIFO");
                free(command);
                memset(buffer, 0, MAX_BUF_SIZE);
                continue;
            }
            memset(buffer, 0, MAX_BUF_SIZE);
            free(command);
        }

         snprintf(buffer, MAX_BUF_SIZE, "----------------------------------------\n");
        if (write(client_fifo_fd, buffer, strlen(buffer)) == -1)
        {
            perror("Failed to write to client FIFO");
            free(buffer);
            close(client_fifo_fd);
            exit(EXIT_FAILURE);
        }
        memset(buffer, 0, MAX_BUF_SIZE);
        

        
       

        snprintf(buffer, MAX_BUF_SIZE, "Finished\n");
        check = write(client_fifo_fd, buffer, strlen(buffer));
        if (check == -1)
        {
            perror("Failed to write to client FIFO");
            free(buffer);
            close(client_fifo_fd);
            exit(EXIT_FAILURE);
        }

        int log_fd = open(LOG_PATH, O_RDONLY);
        if (log_fd == -1)
        {
            perror("Error opening log file");
            free(buffer);
            exit(EXIT_FAILURE);
        }

        while (read(log_fd, buffer, MAX_BUF_SIZE) > 0)
        {
            check = write(client_fifo_fd, buffer, strlen(buffer));
            if (check == -1)
            {
                perror("Failed to write to client FIFO");
                memset(buffer, 0, MAX_BUF_SIZE);
                continue;
            }
            memset(buffer, 0, MAX_BUF_SIZE);
        }

        close(log_fd);
        close(client_fifo_fd);
        free(buffer);
        _exit(0);
    }
}

char **parse_command(const char *command)
{
    // Creates a copy of the command string
    char *buffer = strdup(command);
    if (buffer == NULL)
    {
        printf("Failed to duplicate command string");
    }

    int arg_count = 0;
    char **args = NULL;
    char *token;
    const char *delim = " ";

    strtok(buffer, delim); // "execute"
    strtok(NULL, delim);   // "time"
    strtok(NULL, delim);   // "-u"

    // Process the remaining arguments
    while ((token = strtok(NULL, delim)) != NULL)
    {
        // Realocates the array of arguments
        char **new_args = realloc(args, (arg_count + 2) * sizeof(char *)); // +2 for the new argument and the NULL terminator
        if (new_args == NULL)
        {
            printf("Failed to reallocate arguments array");
            free(args);   // Free the old array
            free(buffer); // Free the buffer
            return NULL;
        }
        args = new_args;
        args[arg_count++] = strdup(token);
    }
    if (args != NULL)
    {
        args[arg_count] = NULL; // Last argument is NULL
    }

    free(buffer); // Free the buffer
    return args;
}

int is_a_simple_process(char *command)
{

    if (strstr(command, "-u") != NULL)
    {
        return 1; // It is a simple process
    }
    else
    {
        return 0; // It is a pipeline process
    }
}

char **parse_command_segment(char *segment)
{
    const int INITIAL_BUF_SIZE = MAX_BUF_SIZE;
    int position = 0, bufsize = INITIAL_BUF_SIZE;
    char **tokens = malloc(bufsize * sizeof(char *));
    char *token;

    if (!tokens)
    {
        fprintf(stderr, "Allocation error\n");
        return NULL;
    }

    token = strtok(segment, " ");
    while (token != NULL)
    {
        tokens[position++] = strdup(token); // DOuble the token
        if (position >= bufsize)
        {
            bufsize += INITIAL_BUF_SIZE;
            tokens = realloc(tokens, bufsize * sizeof(char *));
            if (!tokens)
            {
                fprintf(stderr, "Allocation error\n");
                for (int i = 0; i < position; i++)
                    free(tokens[i]); // Free the memory allocated for the tokens
                return NULL;
            }
        }
        token = strtok(NULL, " ");
    }
    tokens[position] = NULL;

    return tokens;
}

void execute_command_pipeline(char **cmd_args, int input_fd, int output_fd)
{
    if (input_fd != STDIN_FILENO)
    {
        dup2(input_fd, STDIN_FILENO);
        close(input_fd);
    }
    if (output_fd != STDOUT_FILENO)
    {
        dup2(output_fd, STDOUT_FILENO);
        dup2(output_fd, STDERR_FILENO);
        close(output_fd);
    }

    execvp(cmd_args[0], cmd_args);
    perror("execvp");
    exit(EXIT_FAILURE);
}

void execute_pipeline_process(Process process,char* task_output)
{
    int num_pipes = 0;
    char *command = get_command_pipeline(process->command);
    for (int i = 0; command[i] != '\0'; i++)
    {
        if (command[i] == '|')
            num_pipes++;
    }

    int pipefds[num_pipes][2]; // Create a 2D array for pipes
    for (int i = 0; i < num_pipes; i++)
    {
        if (pipe(pipefds[i]) == -1)
        { // Initialize each pipe
            perror("pipe");
            exit(EXIT_FAILURE);
        }
    }

    char *command_segment = strtok(command, "|");
    int i = 0;
    while (command_segment != NULL)
    {
        char *next_command = strtok(NULL, "|");
        pid_t pid = fork();
        if (pid == -1)
        {
            perror("fork");
            exit(EXIT_FAILURE);
        }

        if (pid == 0)
        { // Child process
            // Redirect output to the next command or to the output file if it's the last command
            int output_fd = (next_command) ? pipefds[i][1] : STDOUT_FILENO;
            if (!next_command)
            {
                // Last command, redirect stdout to a file
                char output_file_path[256];
                sprintf(output_file_path, "%staskoutput_%d.txt", task_output , process->id);
                int fd = open(output_file_path, O_WRONLY | O_CREAT | O_TRUNC, 0666);
                if (fd < 0)
                {
                    perror("Failed to open output file");
                    exit(EXIT_FAILURE);
                }
                output_fd = fd;
            }

            execute_command_pipeline(parse_command_segment(command_segment), (i == 0) ? STDIN_FILENO : pipefds[i - 1][0], output_fd);
            // Note: child process should never reach here.
        }

        // Parent closes used fds
        if (i != 0)
        {
            close(pipefds[i - 1][0]);
        }
        if (next_command)
        {
            close(pipefds[i][1]);
        }

        command_segment = next_command;
        i++;
    }

    // Wait for all children to finish
    while (wait(NULL) > 0)
        ;
}

void execute_simple_process(Process process,char* task_output)
{
    char **args = parse_command(process->command); // this memory will be freed by the execvp function
    char file_process[256];                        // format is process_id.txt
    sprintf(file_process, "%staskoutput_%d.txt", task_output, process->id);
    int fd = open(file_process, O_WRONLY | O_CREAT | O_APPEND, 0666);
    if (fd == -1)
    {
        perror("Failed to open log file");
        exit(EXIT_FAILURE);
    }

    dup2(fd, STDOUT_FILENO);
    dup2(fd, STDERR_FILENO);
    close(fd);

    int check = execvp(args[0], args); // exec will free the memory allocated for args array

    if (check == -1)
    {
        perror("Failed to execute command");
        for (int i = 0; args[i] != NULL; i++)
        {
            free(args[i]);
        }
        exit(EXIT_FAILURE);
    }
}