/*
 * Copyright © 2018 Dimonchik0036. All rights reserved.
 */


#include "execute.h"
#include "builtin.h"
#include "terminal.h"

#include <fcntl.h>
#include <wait.h>
#include <signal.h>


#define CHECK_ON_ERROR(expected, actual, message) if ((expected) == (actual)) { perror(message); return CRASH; }


static int execute_parent(JobController *controller,
                          pid_t descendant_pid,
                          Command *command);

static int execute_descendant(CommandLine *command_line, Command *command);

static int execute_conveyor(JobController *controller,
                            CommandLine *command_line);

static void prepare_conveyor(CommandLine *command_line);

static int processing_conveyor_command(CommandLine *command_line,
                                       size_t current_index);

static int execute_conveyor_parent(JobController *controller,
                                   CommandLine *command_line);

static void execute_conveyor_wait(JobController *controller,
                                  CommandLine *command_line,
                                  Command *command);

static void processing_conveyor_parent(CommandLine *command_line,
                                       Command *command);

static int set_infile(char *infile);

static int set_outfile(char *outfile, char addfile);

static int use_dup2(int fd, int fd2, char *error);

static int set_redirects(CommandLine *command_line, Command *command);

static void command_set_background_signal(const Command *command);

static int set_input_terminal();

static int exec_command(JobController *controller,
                        CommandLine *command_line,
                        Command *command);


int execute_command_line(JobController *controller,
                         CommandLine *command_line,
                         ssize_t number_of_commands) {
    if (number_of_commands <= 0) {
        return CONTINUE;
    }

    size_t index_of_command;
    for (index_of_command = 0;
         index_of_command < number_of_commands;
         ++index_of_command) {
        Command *current_command = &command_line->commands[index_of_command];
        command_line->current_index_of_command = index_of_command;

        int exit_code = builtin_exec(controller, current_command);
        switch (exit_code) {
            case EXIT:
                return exit_code;
            case CONTINUE:
                break;
            case STOP:
            default:
                continue;
        }

        if (current_command->flag & IN_PIPE) {
            continue;
        }

        exit_code = exec_command(controller, command_line, current_command);
        switch (exit_code) {
            case EXIT:
            case CRASH:
                return exit_code;
            default:
                break;
        }
    }

    return CONTINUE;
}

static void execute_conveyor_wait(JobController *controller,
                                  CommandLine *command_line,
                                  Command *command) {
    size_t number_of_children = command_line->last_command_in_pipeline -
                                command_line->current_index_of_command + 1;

    pid_t main_pid = command_line->main_process;
    if (command->flag & BACKGROUND) {
        job_controller_add_conveyor(controller, main_pid,
                                    command, JOB_RUNNING, number_of_children);
        return;
    }

    size_t number_of_children_completed = 0;
    while (number_of_children_completed < number_of_children) {
        int status = 0;
        pid_t wait_result = waitpid(-main_pid, &status, WUNTRACED);
        if (wait_result != BAD_RESULT) {
            if (WIFSTOPPED(status)) {
                number_of_children -= number_of_children_completed;
                job_controller_add_conveyor(controller, main_pid, command,
                                            JOB_STOPPED, number_of_children);
                return;
            } else {
                ++number_of_children_completed;
            }
        } else {
            perror("Couldn't wait for child process termination");
        }
    }
}

static int execute_conveyor_parent(JobController *controller,
                                   CommandLine *command_line) {
    size_t index_of_begin_pipeline = command_line->current_index_of_command;
    Command *commands = command_line->commands;
    size_t pipeline_len = command_line->last_command_in_pipeline
                          - index_of_begin_pipeline + 1;

    Command *result_command = command_concat(&commands[index_of_begin_pipeline],
                                             pipeline_len);

    execute_conveyor_wait(controller, command_line, result_command);
    command_free(result_command);

    int exit_code = set_input_terminal();
    if (exit_code == BAD_RESULT) {
        return CRASH;
    }

    return CONTINUE;
}

static void processing_conveyor_parent(CommandLine *command_line,
                                       Command *command) {
    if (command->flag & IN_PIPE) {
        close(command_line->prev_out_pipe);
    }

    close(command_line->pipe_des[1]);
    command_line->prev_out_pipe = command_line->pipe_des[0];
}

static int processing_conveyor_command(CommandLine *command_line,
                                       size_t current_index) {
    Command *current_command = &command_line->commands[current_index];
    int exit_code = pipe(command_line->pipe_des);
    CHECK_ON_ERROR(exit_code, BAD_RESULT, "Couldn't create pipe")

    pid_t pid = fork();
    if (command_line->current_index_of_command == current_index) {
        command_line->main_process = pid;
    }

    switch (pid) {
        case BAD_PID:
            perror("Couldn't create process");
            return CRASH;
        case DESCENDANT_PID:
            return execute_descendant(command_line, current_command);
        default:
            break;
    }

    setpgid(pid, command_line->main_process);
    processing_conveyor_parent(command_line, current_command);
    return CONTINUE;
}

static void prepare_conveyor(CommandLine *command_line) {
    Command *commands = command_line->commands;
    size_t index_of_begin_pipeline = command_line->current_index_of_command;

    size_t index = index_of_begin_pipeline;
    while (commands[index].flag & (IN_PIPE | OUT_PIPE)) {
        ++index;
    }

    size_t last_index = index - 1;

    command_line->last_command_in_pipeline = last_index;
    char background_flag = (char) (commands[last_index].flag & BACKGROUND);
    for (index = index_of_begin_pipeline; index < last_index; ++index) {
        commands[index].flag |= background_flag;
    }
}

static void command_set_background_signal(const Command *command) {
    if (command->flag & BACKGROUND) {
        signal(SIGINT, SIG_IGN);
        signal(SIGQUIT, SIG_IGN);
    } else {
        signal(SIGINT, SIG_DFL);
    }

    signal(SIGTSTP, SIG_DFL);
}

static int execute_conveyor(JobController *controller,
                            CommandLine *command_line) {
    prepare_conveyor(command_line);

    Command *commands = command_line->commands;
    size_t first_index = command_line->current_index_of_command;
    size_t current_index = first_index;
    while (commands[current_index].flag & (IN_PIPE | OUT_PIPE)) {
        int exit_code = processing_conveyor_command(command_line,
                                                    current_index);
        if (exit_code != CONTINUE) {
            return exit_code;
        }

        ++current_index;
    }

    return execute_conveyor_parent(controller, command_line);
}

static int exec_command(JobController *controller,
                        CommandLine *command_line,
                        Command *command) {
    if (command->flag & OUT_PIPE) {
        return execute_conveyor(controller, command_line);
    }

    pid_t pid = fork();
    switch (pid) {
        case DESCENDANT_PID:
            return execute_descendant(command_line, command);
        case BAD_PID:
            perror("Couldn't create process");
            return CRASH;
        default:
            return execute_parent(controller, pid, command);
    }
}

static int execute_parent(JobController *controller,
                          pid_t descendant_pid,
                          Command *command) {
    if (command->flag & BACKGROUND) {
        job_controller_add_job(controller, descendant_pid, command,
                               JOB_RUNNING);
        return CONTINUE;
    }

    int status = 0;
    pid_t wait_result = waitpid(descendant_pid, &status, WUNTRACED);
    if (wait_result != BAD_RESULT) {
        if (WIFSTOPPED(status)) {
            job_controller_add_job(controller, descendant_pid, command,
                                   JOB_STOPPED);
        }
    } else {
        perror("Couldn't wait for child process termination");
    }

    int exit_code = set_input_terminal();
    if (exit_code == BAD_RESULT) {
        return CRASH;
    }

    return CONTINUE;
}

static int set_input_terminal() {
    pid_t pgrp = getpgrp();
    int exit_code = terminal_set_stdin(pgrp);
    if (exit_code == BAD_RESULT) {
        return exit_code;
    }

    return EXIT_SUCCESS;
}

static int execute_descendant(CommandLine *command_line, Command *command) {
    command_set_background_signal(command);

    int exit_code = setpgid(0, (command->flag & (IN_PIPE | OUT_PIPE))
                               ? command_line->main_process
                               : 0);
    CHECK_ON_ERROR(exit_code, BAD_RESULT, "Couldn't set process group ID")

    if (!(command->flag & (BACKGROUND | IN_PIPE))) {
        exit_code = set_input_terminal();
        if (exit_code == BAD_RESULT) {
            return CRASH;
        }
    }

    exit_code = set_redirects(command_line, command);
    if (exit_code == CRASH) {
        return exit_code;
    }

    execvp(command->arguments[0], command->arguments);

    perror("Couldn't execute command");
    return CRASH;
}

static int set_redirects(CommandLine *command_line, Command *command) {
    int exit_code;
    if ((command->flag & IN_FILE) && command->infile) {
        exit_code = set_infile(command->infile);
        if (exit_code == CRASH) {
            return exit_code;
        }
    }

    if ((command->flag & OUT_FILE) && command->outfile) {
        exit_code = set_outfile(command->outfile, command->appfile);
        if (exit_code == CRASH) {
            return exit_code;
        }
    }

    if (command->flag & IN_PIPE) {
        exit_code = use_dup2(command_line->prev_out_pipe, STDIN_FILENO,
                             "Couldn't redirect input");
        if (exit_code == CRASH) {
            return exit_code;
        }
    }

    if (command->flag & OUT_PIPE) {
        exit_code = use_dup2(command_line->pipe_des[1], STDOUT_FILENO,
                             "Couldn't redirect output");
        if (exit_code == CRASH) {
            return exit_code;
        }
    }

    return EXIT_SUCCESS;
}

static int use_dup2(int fd, int fd2, char *error) {
    int exit_code = dup2(fd, fd2);
    if (exit_code == BAD_RESULT) {
        perror(error);
        close(fd);
        return CRASH;
    }
    close(fd);

    return CONTINUE;
}

static int set_infile(char *infile) {
    int input = open(infile, O_RDONLY);
    CHECK_ON_ERROR(input, BAD_RESULT, "Couldn't open input file")

    return use_dup2(input, STDIN_FILENO, "Couldn't redirect input");
}

static int set_outfile(char *outfile, char addfile) {
    int flags = O_WRONLY;
    if (addfile == TRUE) {
        flags |= O_APPEND | O_CREAT;
    } else {
        flags |= O_CREAT | O_TRUNC;
    }

    int output;
    output = open(outfile, flags, (mode_t) 0644);
    CHECK_ON_ERROR(output, BAD_RESULT, "Couldn't open output file")

    return use_dup2(output, STDOUT_FILENO, "Couldn't redirect output");
}
