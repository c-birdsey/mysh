//calder birdsey
//cs315 systems programming 
//assignment 5: mysh 

#include <sys/wait.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

/*
fork(2)
exec(3)
wait(2)
pipe(2)
getline(3)
dup(2)
fgets(3)
strtok(3)

supports:
1. input redirection (<)
2. output redirection (>)
    and output redirection-- appended (>>)
3. piping (|) 
    loosely equivalent to:
        ./foo > temp
        ./foo2 < temp

ORDER for implementation:
    output
    input 
    pipes (single pipe first, then multiple pipes)

behavior:
1. print prompt/new prompt after command 
2. run commands when commands are typed with newline 
3. terminates with user "exit" or EOF (ctrl-D)- DONE
4. support arbitrary number of pipes in a single command 
5. input never exceeds 4096 chars 
6. individual commands will have a max of ten args
*/ 

/* PROTOTYPES */
char ** parse_command(void); 
void * run_command(char **args); 
//int check_exit(char **args);
void *print_args(char **args); 

/*GLOBALS*/
#define INPUT_SIZE 4096

int
main(int argc, char *argv[]) {
    char ** args; 

    while(1) {
        printf("[mysh]$ "); 
        args = parse_command(); 
        //print_args(args); 
        if ((args[0] == NULL) || ((strncmp(args[0], "exit", 1) == 0))) {
            break; 
        } 
        //run_command(args); 
    }
    free(args); 
    printf("\n"); 
    return EXIT_SUCCESS; 
}

char **
parse_command(void) { 
    size_t buf = 0; 
    char *input; 
    char **args = malloc(INPUT_SIZE); 

    //get line from command line  
    if ((getline(&input, &buf, stdin) == -1)) {
        args[0] = NULL; 
        return args; 
    }

    //break input into individual args
    char *token; 
    int count = 0; 
    while((token = strtok_r(input, " ", &input))) {
        args[count] = token; 
        count ++; 
    }

    //seperate pipes and redirections 

    args[count] = NULL;  //NULL identifier at end or args array 
    return args; 
}

void *
run_command(char **args) {


    return NULL; 
}

/*int
check_exit(char **args) {
    char *first_arg = args[0]; 

    //check against exit string 
    if ((strncmp(first_arg, "exit", 1) == 0)) {
        return -1; 
    } else {
        return 0;
    }
}*/

void *
print_args(char **args) { 
    int count = 0; 
    while(args[count]) {
        printf("Arg %d: %s\n", count, args[count]); 
        count ++; 
    }
    return NULL; 
}
