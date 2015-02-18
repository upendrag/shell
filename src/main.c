#include <stdio.h>
#include <stdlib.h>

#define QUIT_CMD "quit"
#define MAX_CMD_LEN 512

inline void print_error(const char *message)
{
    fprintf(stderr, "shell: %s\n", message);
}

inline void print_prompt()
{
    fprintf(stdout, "shell-prompt> ");
}

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
    while(*word == ' ') word++;
    char *tmp = word;
    while(*tmp != ' ') tmp++;
    *tmp = 0;
    return word;
}

int run_in_interactive_mode()
{
    char cmd_text[MAX_CMD_LEN];
    char *cmd;
    while (1) {
        print_prompt();
        fgets(cmd_text, MAX_CMD_LEN, stdin);
        remove_end_of_line(cmd_text);
        cmd = trim_word(cmd_text);
        if (compare_text(cmd, QUIT_CMD) == 0) {
            break;  /* exit prompt  */
        }
    } /* run till 'quit' is called  */
    return 0;
}

int run_in_batch_mode(const char *file)
{
    return 0;
}

int main(int argc, char *argv[])
{
    int ret_val = 0;
    if (argc > 2) {
        print_error("Invalid arguments");
        print_error("Usage: shell <file>");
        exit(-1);
    }

    if (argc == 1) {
        ret_val = run_in_interactive_mode();
    } /* if no batch file specified */
    else {
        ret_val = run_in_batch_mode(*argv);
    } /* if batch file specified  */
    
    return ret_val;
}