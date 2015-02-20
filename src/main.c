#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <pthread.h>

#define QUIT_CMD "quit"
#define MAX_CMD_LEN 512
#define ARGS_DELIM " "
#define CMD_DELIM ";"

int compare_text(const char *t1, const char *t2)
{
    int result = 0;

    while (*t1 != 0 && *t2 != 0 ) {
        if (*t1 > *t2) {
            result = 1;
            break;
        }
        else if (*t1 < *t2) {
            result = -1;
            break;
        }
        t1++, t2++;
    }
    if (result == 0 && (*t1 != 0 || *t2 != 0)) {
        result = *t1 != 0? 1: -1;
    } /* if both texts are not terminated yet  */

    return result;
}

void remove_end_of_line(char *line)
{
    int i = 0;

    while (line[i] != '\n' && i < MAX_CMD_LEN) i++;
    line[i] = 0;
}

char* trim_word(char *word)
{
    while (*word == ' ' && *word != 0) word++;
    if (*word == 0) return NULL;
    char *tmp = word;
    while (*tmp != 0) tmp++;
    do {
        tmp--;
    } while (*tmp == ' ');
    *(tmp + 1) = 0;
    return word;
}

void start_new_program(void *command)
{
    pid_t pid;
    char *cmd = (char *)command;

    pid = fork();
    if (pid > 0) {
        int status;
        waitpid(pid, &status, 0);
        if (status) {
            fprintf(stderr, "Unexpected termination: pid: %d\n", pid);
        }
    } /* parent  */
    else if(pid == 0) {
        int ret_val = EXIT_SUCCESS;
        char **argv = NULL;
        char *arg = strtok(cmd, ARGS_DELIM);
        int i = 0;
        while (arg != NULL) {
            if (!argv) argv = (char **)malloc(sizeof(char *));
            else argv = (char **)realloc(argv, sizeof(char *));
            argv[i++] = arg;
            arg = strtok(NULL, ARGS_DELIM);
        }
        /* NULL termination of arguments */
        argv = (char **)realloc(argv, sizeof(char *));
        argv[i] = NULL;

        if (!execvp(*argv, argv)) {
            perror("shell:");
            ret_val = EXIT_FAILURE;
        }
        fprintf(stderr, "command not found: %s\n", *argv);
        /* free argv */
        free(argv);
        exit(ret_val);
    } /* child  */
    else {
        perror("shell: start_new_program");
    } /* error  */
}

char** get_commands(char *cmd_text, int *num_commands)
{
    char **commands = NULL;
    char *cmd, *trimmed_cmd;
    int i = 0;

    cmd = strtok(cmd_text, CMD_DELIM);
    while (cmd != NULL) {
        trimmed_cmd = trim_word(cmd);
        if (trimmed_cmd) {
            if (!commands) commands = (char **) malloc(sizeof(char *));
            else commands = (char **) realloc(commands, sizeof(char *));
            commands[i++] = trimmed_cmd;
        }
        cmd = strtok(NULL, CMD_DELIM);
    }
    *num_commands = i;
    return commands;
}

void execute_commands(char **commands, int num_commands)
{
    int do_quit = -1;
    int i = 0, thread_code;
    pthread_t threads[num_commands]; 

    for (i = 0; i < num_commands; i++) {
        if (strcmp(commands[i], QUIT_CMD) == 0) {
            do_quit = i;            
            break;
        }
    }
    for (i = 0; i < num_commands; i++) {
        if (i == do_quit) continue;        

        thread_code = pthread_create(&threads[i], NULL, 
            (void *) start_new_program, commands[i]);
        if (thread_code) {
            fprintf(stderr, "shell: Error creating thread; return code: %d\n", 
                    thread_code);
        }
    }
    
    for (i = 0; i < num_commands; i++) {
        if(i == do_quit) continue;
        if (pthread_join(threads[i], NULL)) perror("shell");
    }
    
    if (do_quit > -1) {
        free(commands);
        /* there is a quit command */
        exit(EXIT_SUCCESS);
    }
}

void execute_line(char *cmd_text)
{
    char **commands = NULL;
    int num_commands = 0;

    remove_end_of_line(cmd_text);
    commands = get_commands(cmd_text, &num_commands);
    if(num_commands)
        execute_commands(commands, num_commands);
    free(commands);
}

int run_in_interactive_mode()
{
    char cmd_text[MAX_CMD_LEN];

    while (1) {
        fprintf(stdout, "shell-prompt> ");
        if (!fgets(cmd_text, MAX_CMD_LEN, stdin)) {
            fprintf(stdout, "\n");
            return EXIT_SUCCESS;
        }
        execute_line(cmd_text);
    } /* run till 'quit' is called  */
    return 0;
}

int run_in_batch_mode(const char *file)
{
    char cmd_text[MAX_CMD_LEN];
    FILE *fp;

    fp = fopen(file, "r");
    if (!fp) {
        fprintf(stderr, "shell: Error opening file %s\n", file);
        return EXIT_FAILURE;
    }
    while (fgets(cmd_text, MAX_CMD_LEN, fp)) {
        fprintf(stdout, "shell: execute: %s", cmd_text);
        execute_line(cmd_text);
    }
    fclose(fp);
    return 0;
}

int main(int argc, char *argv[])
{
    int ret_val = 0;

    if (argc > 2) {
        fprintf(stderr, "shell: Invalid arguments\n");
        fprintf(stderr, "shell: Usage: shell [script_file]\n");
        exit(EXIT_FAILURE);
    }

    if (argc == 1) {
        ret_val = run_in_interactive_mode();
    } /* if no batch file specified */
    else {
        ret_val = run_in_batch_mode(argv[1]);
    } /* if batch file specified  */
    
    return ret_val;
}
