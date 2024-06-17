#ifndef COMMAND_INTERPRETER_H
#define COMMAND_INTERPRETER_H

char **command_interpreter(char *command);
void status_writer(char *client_fifo_path, Process processes[], int active_tasks, Scheduler scheduler);
char **parse_command(const char *command);
char *get_command(char *command);
char *get_command_pipeline(char *command);
int is_a_simple_process(char *command);
char **parse_command_segment(char *segment);
void execute_pipeline_process(Process process, char* task_path);
void execute_simple_process(Process process, char* task_path);
#endif