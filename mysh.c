//calder birdsey
//cs315 systems programming 
//assignment 5: mysh 

#include <sys/wait.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h> 
#include <fcntl.h>

/*
fork(2)
exec(3)
wait(2)
pipe(2)
getline(3)
dup(2)
fgets(3)
strtok(3)
*/ 

/* PROTOTYPES */
void parse_command(char *input); 
void *run_command(char **args); 
void *print_args(char **args); 
//void cmd_handler(char ** args); 
void no_ops_execute(char **args); 
void input_redir(char **cmd, char *input); 
void output_redir(char **cmd, char *output, int append_flag); 
void pipe_handler(char ***pipe_args, int pipe_count); 
int check_args(char **args); 
int count_pipes(char *args); 

/*STRUCT*/
struct input_cmd {
    char **args1;
    char **args2;  
    char *operator;
    struct input_cmd *next_cmd;  
}; 

struct client *first_client;

/*GLOBALS*/
#define INPUT_SIZE 4096

int
main(int argc, char *argv[]) {
    while(1) {
        printf("[mysh]$ "); 
        char *input; 
        size_t buf = 0;
        //get line from command line and check for exit/blank input, then parse
        if ((getline(&input, &buf, stdin) == -1) || (strncmp(input, "exit\n", 5) == 0)) {
            break;
        }
        if(strcmp(input, "\n") == 0) { 
            continue; 
        }
        //remove newline char and parse
        strtok(input, "\n"); 
        parse_command(input); 
    }
    //free(args); 
    printf("\n"); 
    return EXIT_SUCCESS; 
}

void
parse_command(char *input) { 
    char *arg; 
    char **args = malloc(INPUT_SIZE);  
    int count; 

    //break up by pipe/io delimiters 
    //handle pipes 
    if(strchr(input, '|')) { 
        char ***pipe_args = malloc(INPUT_SIZE);  
        int pipe_number = count_pipes(input); 
        count = 0;  
        char *pipe_arg; 
        while((pipe_arg = strtok_r(input, "|", &input))) {
            int i = 0; 
            char **cmd_args = malloc(INPUT_SIZE); 
            while((arg = strtok_r(pipe_arg, " ", &pipe_arg))) {
                cmd_args[i] = arg; 
                i ++; 
            }
            pipe_args[count] = cmd_args;
            count ++; 
        }
        pipe_handler(pipe_args, pipe_number); 
        return; 
    //handle input redirection
    } else if(strchr(input, '<')) {
        count = 0;  
        while((arg = strtok_r(input, "<", &input))) {
            args[count] = arg; 
            count ++; 
        }
        count = 0; 
        char *cmd_arg; 
        char **cmd_args = malloc(INPUT_SIZE); 
        while((cmd_arg = strtok_r(args[0], " ", &args[0]))) {
            cmd_args[count] = cmd_arg; 
            count ++; 
        }
        char *input_file = strtok(args[1], " "); 
        input_redir(cmd_args, input_file); 
        free(cmd_arg); 
        return; 
    //print output redirection 
    } else if(strstr(input, ">")) {
        int append_flag = 0; 
        if(strstr(input, " >> ")) {
            append_flag = 1; 
        }
        count = 0; 
        while((arg = strtok_r(input, ">", &input))) {
            args[count] = arg; 
            count ++; 
        }
        count = 0; 
        char *cmd_arg; 
        char **cmd_args = malloc(INPUT_SIZE); 
        while((cmd_arg = strtok_r(args[0], " ", &args[0]))) {
            cmd_args[count] = cmd_arg; 
            count ++; 
        }
        char *output = strtok(args[1], " ");
        output_redir(cmd_args, output, append_flag); 
        free(cmd_arg); 
        return; 
    //handle no ops 
    } else {
        count = 0; 
        while((arg = strtok_r(input, " ", &input))) {
            args[count] = arg; 
            count ++; 
        }
        no_ops_execute(args);    
    }    
    return; //args
}

/*void 
cmd_handler(char ** args) {
    char *operators[] = {"|", "<", ">", ">>"};
    char** cmd_args; 
  
    
    //parse for i/o delimiters 
    //strtok(args, "<"); 

    
    int i = 0; 
    while(args[i]) {
        if ((strncmp(args[i], "<", 1) == 0)) {

        }
    }
    iterate through args
    for(int i = 0; i <= sizeof(args);i++) {
        //check for i/o  and pipe operators 
        for(int j = 0; j <= 3; j++) {
            if ((strncmp(args[i], operators[j], 1) == 0)) {
                if (j == 0) { //pipe
                    printf("piping"); 
                    exit(1); 
                } else if (j == 1) { //input redirection 
                    input_redir(cmd_args, args[i + 1]); 
                } else if (j == 2) { //output redirection w/trunc
                    output_redir_trunc(cmd_args, args[i + 1]);
                } else if (j == 3) { //output redirection w/append 
                    output_redir_app(cmd_args, args[i + 1]);
                }
            }
            continue; 
        }
        //if token does not equal any of the operators, meaning its cmd or variable, add it to cmd_args 
        cmd_args[i] =  args[i];
    }
    //if outside for loop finishes without calling operator functions, 
}*/

void 
no_ops_execute(char **args) {
    int exit_value;
    if(fork() == 0) {
        execvp(args[0], args);
        printf("Error: command not found\n"); 
        exit(1); 
    }
    wait(&exit_value); 
    return; 
}

void 
input_redir(char **cmd, char *input) {
    int exit_value;  
    int stdin_restore = dup(STDIN_FILENO); //saving stdout to be restored 
    int file_fd = open(input, O_RDONLY); 
    dup2(file_fd, 0); 
    if(fork() == 0) {
        execvp(cmd[0], cmd); 
        exit(1); 
    }
    wait(&exit_value); 
    dup2(stdin_restore, STDIN_FILENO); 
    close(stdin_restore); 
    close(file_fd);
    return; 
}

void 
output_redir(char **cmd, char *output, int append_flag) {
    int exit_value;  
    int stdout_restore = dup(STDOUT_FILENO); //saving stdout to be restored 
    int flags; 
    if(append_flag == 1) {
        flags = (O_WRONLY | O_CREAT | O_APPEND); 
    } else {
        flags = (O_WRONLY | O_CREAT); 
    }
    int file_fd = open(output, flags); 
    dup2(file_fd, 1); 
    if(fork() == 0) {
        execvp(cmd[0], cmd); 
        exit(1); 
    }
    wait(&exit_value); 
    dup2(stdout_restore, STDOUT_FILENO); 
    close(stdout_restore); 
    close(file_fd);
    return; 
}

void 
pipe_handler(char ***cmds, int pipe_count) {
    int exit_value;  
    int input = 0; 
    int output = 1; 
    int i = 0; 
    int pipe_fd[2]; 
    if (pipe(pipe_fd) == -1) {
        perror("pipe"); 
        exit(EXIT_FAILURE); 
    }

    while(i <= pipe_count) {
        //first pipe command 
        if (i == 0) {
            if(fork() == 0) {
                close(1); 
                close(pipe_fd[0]);
                dup2(pipe_fd[1], 1);
                execvp(cmds[i][0], cmds[i]); 
                exit(1); 
            } 
            wait(&exit_value); 
            i ++; 
            continue; 
        } 
        //final command 
        if (i == pipe_count) {
            if(fork() == 0) {
                close(pipe_fd[1]);
                dup2(pipe_fd[0], input);
                dup2(pipe_fd[1], 1); 
                execvp(cmds[1][0], cmds[1]); 
                exit(1); 
            }
            wait(&exit_value); 
            break; 
        }
        //iterating through commands 2 - (n-1)
        if(fork() == 0) {
            close(0); 
            close(1); 
            input = 1 - input; 
            output = 1 - output; 
            dup2(pipe_fd[0], input);
            dup2(pipe_fd[1], output);
            execvp(cmds[i][0], cmds[i]); 
            exit(1); 
        } 
        wait(&exit_value); 
        i ++;  
    }
    /*
    if(fork() == 0) {
        close(pipe_fd[0]);
        dup2(pipe_fd[1], 1);
        execvp(cmds[i][0], cmds[i]); 
        //parse_command(cmds[0]); 
        exit(1); 
    } 
    if(fork() == 0) {
        close(pipe_fd[1]);
        dup2(pipe_fd[0], 0);
        execvp(cmds[1][0], cmds[1]); 
        //parse_command(cmds[1]); 
        exit(1); 
    }*/ 
    close(pipe_fd[0]); 
    close(pipe_fd[1]);
    return; 
}

int 
check_args(char **args) {
    char *operators[] = {"|", "<", ">", ">>"};
    int count = 0; 
    while(args[count]) {
        for(int j = 0; j <= 3; j++) {
            if ((strncmp(args[count], operators[j], 1) == 0)) {
                return -1; 
            }
        }
    count ++; 
    }
    return 0; 
}

int 
count_pipes(char *args) {
    int count = 0; 
    for (int i=0; i < strlen(args); i++) {
        count += (args[i] == '|');
    }
    return count; 
}

/******** FOR TESTING **********/
void *
print_args(char **args) { 
    int count = 0; 
    while(1) {
        printf("Arg %d: %s\n", count, args[count]); 
        if(args[count] == NULL) {
            break;
        }
        count ++; 
    }
    printf("\n"); 
    return NULL; 
}