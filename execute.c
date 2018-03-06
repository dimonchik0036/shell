/*
 * Copyright © 2018 Dimonchik0036. All rights reserved.
 */


#include "execute.h"
#include "builtin.h"
#include "terminal.h"

#include <fcntl.h>
#include <wait.h>
#include <signal.h>


static int execute_parent(JobController *controller,
                          pid_t descendant_pid,
                          Command *command);

static int execute_descendant(CommandLine *command_line, Command *command);

static int execute_conveyor(JobController *controller,
                            CommandLine *command_line);

static int execute_conveyor_main_parent(JobController *controller,
                                        CommandLine *command_line,
                                        pid_t descendant_pid);

static void execute_conveyor_parent(CommandLine *command_line,
                                    Command *command,
                                    size_t index_of_begin_pipeline,
                                    size_t index_of_end_pipeline);

static int execute_conveyor_descendant(CommandLine *command_line);

static int set_infile(char *infile);

static int set_outfile(char *outfile, char addfile);

static int use_dup2(int fd, int fd2, char *error);

static int set_redirects(CommandLine *command_line, Command *command);

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

static int execute_conveyor_main_parent(JobController *controller,
                                        CommandLine *command_line,
                                        pid_t descendant_pid) {
    Command *commands = command_line->commands;
    size_t index_of_begin_pipeline = command_line->current_index_of_command;

    size_t pipeline_len = 0;
    size_t index = index_of_begin_pipeline;
    while (commands[index].flag & (IN_PIPE | OUT_PIPE)) {
        ++pipeline_len;
        ++index;
    }

    Command *result_command = command_concat(&commands[index_of_begin_pipeline],
                                             pipeline_len);
    int answer = execute_parent(controller, descendant_pid, result_command);
    command_free(result_command);
    return answer;
}

static void execute_conveyor_parent(CommandLine *command_line,
                                    Command *command,
                                    size_t index_of_begin_pipeline,
                                    size_t index_of_end_pipeline) {
    if (command->flag & IN_PIPE) {
        close(command_line->prev_out_pipe);
    }

    close(command_line->pipe_des[1]);
    command_line->prev_out_pipe = command_line->pipe_des[0];
    if (command->flag & OUT_PIPE) {
        return;
    }

    size_t number_of_children = index_of_end_pipeline - index_of_begin_pipeline;
    size_t number_of_children_completed = 0;
    while (number_of_children_completed <= number_of_children) {
        int status = 0;
        pid_t wait_result = waitpid(0, &status, WUNTRACED);
        if (wait_result != BAD_RESULT) {
            if (WIFEXITED(status)) {
                ++number_of_children_completed;
            }
        } else {
            perror("Couldn't wait for child process termination");
        }
    }
}

static int execute_conveyor_descendant(CommandLine *command_line) {
    signal(SIGINT, SIG_DFL);
    signal(SIGTSTP, SIG_DFL);
    int exit_code = setpgid(0, 0);
    if (exit_code == BAD_RESULT) {
        perror("Couldn't set process group ID");
        return CRASH;
    }

    Command *commands = command_line->commands;
    size_t index_of_begin_pipeline = command_line->current_index_of_command;

    size_t current_index = index_of_begin_pipeline;
    while (commands[current_index].flag & (IN_PIPE | OUT_PIPE)) {
        Command *current_command = &commands[current_index];
        exit_code = pipe(command_line->pipe_des);
        if (exit_code == BAD_RESULT) {
            perror("Couldn't create pipe");
            return CRASH;
        }

        pid_t pid = fork();
        switch (pid) {
            case BAD_PID:
                perror("Couldn't create process");
                return CRASH;
            case DESCENDANT_PID:
                return execute_descendant(command_line, current_command);
            default:
                execute_conveyor_parent(command_line, current_command,
                                        index_of_begin_pipeline, current_index);
        }

        ++current_index;
    }

    pid_t pgid = getpgid(getppid());
    exit_code = terminal_set_stdin(pgid);
    if (exit_code == BAD_RESULT) {
        return CRASH;
    }

    return EXIT;
}

static int execute_conveyor(JobController *controller,
                            CommandLine *command_line) {
    pid_t pid = fork();
    switch (pid) {
        case BAD_PID:
            perror("Couldn't create process");
            return CRASH;
        case DESCENDANT_PID:
            return execute_conveyor_descendant(command_line);
        default:
            return execute_conveyor_main_parent(controller, command_line, pid);
    }
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
    signal(SIGINT, SIG_DFL);
    signal(SIGTSTP, SIG_DFL);

    int exit_code = setpgid(0, (command->flag & (IN_PIPE | OUT_PIPE))
                               ? getppid()
                               : 0);
    if (exit_code == BAD_RESULT) {
        perror("Couldn't set process group ID");
        return CRASH;
    }

    if (!(command->flag & (BACKGROUND | IN_PIPE))) {
        exit_code = set_input_terminal();
        if (exit_code == BAD_RESULT) {
            return CRASH;
        }
    }

    if (command->flag & BACKGROUND) {
        signal(SIGINT, SIG_IGN);
        signal(SIGQUIT, SIG_IGN);
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
    if (input == BAD_RESULT) {
        perror("Couldn't open input file");
        return CRASH;
    }

    return use_dup2(input, STDIN_FILENO, "Couldn't redirect input");
}

static int set_outfile(char *outfile, char addfile) {
    int flags = O_WRONLY;
    if (addfile) {
        flags |= O_APPEND | O_CREAT;
    } else {
        flags |= O_CREAT | O_TRUNC;
    }

    int output;
    output = open(outfile, flags, (mode_t) 0644);

    if (output == BAD_RESULT) {
        perror("Couldn't open output file");
        return CRASH;
    }

    return use_dup2(output, STDOUT_FILENO, "Couldn't redirect output");;
}
