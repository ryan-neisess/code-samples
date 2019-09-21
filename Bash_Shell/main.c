#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h> // O_RDONLY, O_WRONLY, O_CREAT, O_APPEND
#include <unistd.h>

void clear_user_args(char * user_args[], int i);
void exec_user_cmd(char * user_input, char path_dirs[12][144], char * env[]);
void eval_user_cmd(char * user_input, char path_dirs[12][144], char * env[]);

// drives rn_sh program
int main(int argc, char * argv[], char * env[]) {
    char ** env_ptr = env;

    /*while (*env_ptr != NULL) { // for debugging/reference
        printf("%s\n", *env_ptr);
        env_ptr++;
    }
    putchar('\n');
    env_ptr = env;*/
    
    // get HOME and PATH vars from env
    char env_HOME[144] = "";
    char env_PATH[144] = "";

    while (*env_HOME == '\0' || *env_PATH == '\0') {
        if (strncmp(*env_ptr, "HOME", 4) == 0) {
            strcpy(env_HOME, *env_ptr + 5);
        }
        else if (strncmp(*env_ptr, "PATH", 4) == 0) {
            strcpy(env_PATH, *env_ptr + 5);
        }
        env_ptr++;
    }

    // for debugging/reference
    /*printf("$HOME: %s\n", env_HOME);
    printf("$PATH: %s", env_PATH);
    putchar('\n');*/

    // tokenize PATH var
    char path_dirs[12][144] = {{'\0'}};
    int i = 0;
    char * curr_dir = strtok(env_PATH, ":");

    while (curr_dir != NULL) {
        strcpy(path_dirs[i++], curr_dir);
        curr_dir = strtok(NULL, ":");
    }
    //path_dirs[i][0] = '\0';

    // begin execution loop
    char user_input[144] = "";

    do {
        printf("rn_sh $ ");
        fgets(user_input, 143, stdin);
        user_input[strlen(user_input) - 1] = '\0'; // replace \n
        
        if (strncmp(user_input, "cd", 2) == 0 && 
            (user_input[2] == '\0' || user_input[2] == ' ')) {
            
            if (user_input[2] == ' ') {
                char * new_dir = strtok(user_input, " ");
                new_dir = strtok(NULL, " ");
                chdir(new_dir);
            }
            else {
                chdir(env_HOME);
            }
        }
        else if (strncmp(user_input, "exit", 4) == 0) {
            exit(1);
        }
        else { // non-simple commands, will parse and attempt execution
            eval_user_cmd(user_input, path_dirs, env);
        }
    } while (1);

    return 0;
}

// frees memory and sets i, i + 1 args to NULL
void clear_user_args(char * user_args[], int i) {
    free(user_args[i]);
    user_args[i] = NULL;
    free(user_args[i + 1]);
    user_args[i + 1] = NULL;
}

// handles I/O redirection and will issue the call to execve() if cmd is valid
void exec_user_cmd(char * user_input, char path_dirs[12][144], char * env[]) {
    // tokenize user args
    char * user_args[24] = { NULL };
    char * curr_arg = strtok(user_input, " ");
    int i = 0;

    while (curr_arg != NULL) {
        user_args[i] = (char *)malloc(sizeof(char) * strlen(curr_arg) + 1);
        strcpy(user_args[i++], curr_arg);
        curr_arg = strtok(NULL, " ");
    }

    // check for file redirection
    for(--i; i >= 0; i--) {
        if (strcmp(user_args[i], "<") == 0) { // redirect input
            close(0); // close stdin to keyboard
            open(user_args[i + 1], O_RDONLY);

            // must "erase" redirection commands from input
            clear_user_args(user_args, i);
        }
        else if (strcmp(user_args[i], ">") == 0) {
            close(1); // close stdout to console
            open(user_args[i + 1], O_WRONLY | O_TRUNC | O_CREAT, 0644);

            clear_user_args(user_args, i);
        }
        else if (strcmp(user_args[i], ">>") == 0) {
            close(1);
            // when calling open(), MUST specify one of O_RDONLY, O_WRONLY, or O_RDWR
            // O_APPEND is an additional option to be bitwise OR'ed
            open(user_args[i + 1], O_RDWR | O_APPEND | O_CREAT, 0644);

            clear_user_args(user_args, i);
        }
    }

    // search for command in bin paths
    char bin_path[64] = "";

    for (i = 0; i < 12 && path_dirs[i] != '\0'; i++) {
        strcpy(bin_path, path_dirs[i]); // try the next path
        strcat(bin_path, "/"); // add '/'
        strcat(bin_path, user_args[0]); // add cmd name

        execve(bin_path, user_args, env);
    }
    // try curr directory
    execve(user_args[0], user_args, env);

    exit(100);
}

// recursively checks for pipes and passes along execution appropriately
void eval_pipe_cmd(char * user_input, char path_dirs[12][144], char * env[]) {
    // check for pipes
    int i = 0;
    for ( ; user_input[i] != '\0' && user_input[i] != '|'; i++) {
        // finds either first pipe or null char
    }
    if (user_input[i] == '\0') { // no pipe, execute normally
        exec_user_cmd(user_input, path_dirs, env);
        return; // this return should theoretically never execute
    }
    // else, pipe found
    user_input[i - 1] = '\0'; // replace space before pipe with '\0'
    char * tail_cmd = &user_input[i + 2];
    int pd[2];

    pipe(pd); // creates a pipe
    // pd[0] for reading from the pipe
    // pd[1] for writing to the pipe

    int pipe_pid = fork(); // note: child has same pd[0] and pd[1]

    // parent will be pipe writer, child will be pipe reader

    if (pipe_pid) { // pipe parent process, pid has child process id
        close(pd[0]); // writer must close pd[0]

        close(1);
        dup(pd[1]); // replace 1 with pd[1]
        close(pd[1]); // close pd[1] after it replaces 1

        // go on to call execve() to change image to cmd1/head
        exec_user_cmd(user_input, path_dirs, env);

        int pipe_status = 0;
        pipe_pid = wait(&pipe_status);
        printf("\n  child process %d terminated with code %d\n", pipe_pid, pipe_status);
    }
    else { // pipe_pid == 0, pipe child process
        close(pd[1]); // reader must close pd[1]

        close(0);
        dup(pd[0]); // replace 0 with pd[0]
        close(pd[0]); // close pd[0] after it replaces 0

        // first recursively check for more pipes
        // then go on to call execve() to change image to cmd2/tail
        eval_pipe_cmd(tail_cmd, path_dirs, env);
    }
}

// forks process and passes along execution
void eval_user_cmd(char * user_input, char path_dirs[12][144], char * env[]) {
    int pid = fork(); // returns child pid if parent, 0 if child
    
    if (pid < 0) { // no more PROCs in kernel
        perror("fork failed");
        exit(1);
    }
    else if (pid > 0) { // parent process
        int status = 0;
        pid = wait(&status); // child's exit code will be stored in status
        printf("\n  child process %d terminated with code %d\n", pid, status);
    }
    else { // child process
        // check for pipes and continue execution
        eval_pipe_cmd(user_input, path_dirs, env);
    }
}