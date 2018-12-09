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

/* PROTOTYPES */
void pipe_parse(char *input); 
void io_parse(char *input); 
void no_ops_execute(char **args); 
void input_redir(char **cmd, char *input); 
void output_redir(char **cmd, char *output, int append_flag);  
void pipe_handler(char **pipe_args, int pipe_count); 
void io_redir(char **cmd, char *input, char *output, int append_flag); 
int count_pipes(char *args); 
void print_args(char **args); 

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
        pipe_parse(input); 
    }
    printf("\n"); 
    return EXIT_SUCCESS; 
}

void
pipe_parse(char *input) {
    char *arg; 
    char **args = malloc(INPUT_SIZE);  
    int count; 

    //parse first for pipes and pass to pipe_handler: if no pipes, pass input to io_parse 
    if(strchr(input, '|')) { 
        int pipe_number = count_pipes(input); 
        count = 0;  
        while((arg = strtok_r(input, "|", &input))) {
            args[count] = arg; 
            count ++; 
        }
        pipe_handler(args, pipe_number); 
        free(args); 
    } else {
        io_parse(input); 
    }
    return; 
}

void 
io_parse(char * input) {
    char *arg; 
    char **args = malloc(INPUT_SIZE);  
    int io_flag; 
    int append_flag = 0; 
    int count = 0; 
    
    if((strchr(input, '<')) || (strstr(input, ">"))) {
        //set append flag first 
        if(strstr(input, " >> ")) {
            append_flag = 1; 
        }
        //handle both input and output redir
        if((strchr(input, '<')) && (strstr(input, ">"))) {
            while((arg = strtok_r(input, "<>", &input))) {
                args[count] = arg; 
                count ++; 
            }
            io_flag = 2; 
        //handles input redir
        } else if(strchr(input, '<')) {
            while((arg = strtok_r(input, "<", &input))) {
                args[count] = arg; 
                count ++; 
            }
            io_flag = 1; 
        //handles output redir
        } else if(strstr(input, ">")) {
            while((arg = strtok_r(input, ">", &input))) {
                args[count] = arg; 
                count ++; 
            }
            io_flag = 0; 
        }
        //parse out io files and cmds
        count = 0; 
        char *cmd_arg; 
        char **cmd_args = malloc(INPUT_SIZE); 
        while((cmd_arg = strtok_r(args[0], " ", &args[0]))) {
            cmd_args[count] = cmd_arg; 
            count ++; 
        }
        char *io_file = strtok(args[1], " "); 
        //call appropriate function 
        if(io_flag == 2) {
            char *io_file2 = strtok(args[2], " "); 
            io_redir(cmd_args, io_file, io_file2, append_flag); 
        } else if(io_flag == 1) {
            input_redir(cmd_args, io_file); 
        } else {
            output_redir(cmd_args, io_file, append_flag); 
        }
        free(cmd_arg); 
    //handles no ops 
    } else {
        count = 0; 
        while((arg = strtok_r(input, " ", &input))) {
            args[count] = arg; 
            count ++; 
        }
        no_ops_execute(args);    
    }    
    free(args); 
    return; 
}

void 
no_ops_execute(char **args) {
    int exit_value;
    pid_t pid; 

    pid = fork(); 
    if (pid == -1) {
        perror("fork"); 
        return; 
    } else if(pid == 0) {
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
    int stdin = dup(0);  
    int file_fd = open(input, O_RDONLY); 
    dup2(file_fd, 0); 

    pid_t pid; 
    pid = fork(); 
    if (pid == -1) {
        perror("fork"); 
        return; 
    } else if(pid == 0) {
        execvp(cmd[0], cmd); 
        perror("execvp"); 
        exit(1); 
    }
    wait(&exit_value); 
    dup2(stdin, 0); 
    close(stdin); 
    close(file_fd);
    return; 
}

void 
output_redir(char **cmd, char *output, int append_flag) {
    int exit_value;  
    int flags; 
    int stdout = dup(1);
    if(append_flag == 1) {
        flags = (O_WRONLY | O_CREAT | O_APPEND); 
    } else {
        flags = (O_WRONLY | O_CREAT); 
    }
    int file_fd = open(output, flags); 
    dup2(file_fd, 1); 

    pid_t pid; 
    pid = fork(); 
    if (pid == -1) {
        perror("fork"); 
        return; 
    } else if(pid == 0) {
        execvp(cmd[0], cmd); 
        perror("execvp"); 
        exit(1); 
    }
    wait(&exit_value); 
    dup2(stdout, 1); 
    close(stdout); 
    close(file_fd);
    return; 
}

void 
pipe_handler(char **cmds, int pipe_count) {
    int exit_value;  
    int infd; 
    int pipefd[2]; 

    //loop through pipe commands 
    for (int i = 0; i <= pipe_count; i++) {
        //create new pipe for cmd i 
        if (pipe(pipefd) == -1) { 
            perror("pipe"); 
            exit(EXIT_FAILURE); 
        }
        //fork child to handle cmd 
        pid_t pid; 
        pid = fork(); 
        if (pid == -1) {
            perror("fork"); 
            return; 
        } else if(pid == 0) {
            //for all but first cmd, connect stdin with pipefd[0]
            if(i != 0) { 
                dup2(infd, 0); 
            }
            //for all but last cmd, connect stdout with pipefd[1] 
            if (i != pipe_count) { 
                dup2(pipefd[1], 1); 
            }
            io_parse(cmds[i]); 
            exit(1); 
        } else {
            //wait and store pipefd[0] for next iteration 
            wait(&exit_value); 
            infd = pipefd[0]; 
            close(pipefd[1]); 
        }
    }
}

void
io_redir(char **cmd, char *input, char *output, int append_flag) {
    int exit_value;  
    int flags; 

    //set flags
    if(append_flag == 1) {
        flags = (O_WRONLY | O_CREAT | O_APPEND); 
    } else {
        flags = (O_WRONLY | O_CREAT); 
    }
    
    //manipulate fds
    int stdout = dup(1);
    int stdin = dup(0); 
    int infile_fd = open(input, O_RDONLY); 
    int outfile_fd = open(output, flags); 
    dup2(infile_fd, 0);
    dup2(outfile_fd, 1);

    //fork and execute cmd 
    pid_t pid; 
    pid = fork(); 
    if (pid == -1) {
        perror("fork"); 
        return; 
    } else if(pid == 0) {
        execvp(cmd[0], cmd); 
        perror("execvp"); 
        exit(1); 
    }
    wait(&exit_value); 

    //clean up 
    dup2(stdin, 0); 
    dup2(stdout, 1); 
    close(stdin); 
    close(stdout); 
    close(infile_fd);
    close(outfile_fd);
    return;
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
void
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
    return; 
}