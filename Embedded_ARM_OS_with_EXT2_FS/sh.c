#include "ucode.c"

// #include <stdio.h>
// #include <stdlib.h>
// #include <string.h>
// #include <fcntl.h> // O_RDONLY, O_WRONLY, O_CREAT, O_APPEND
// #include <unistd.h>

void clear_user_args(char * user_args[], int i);
// void exec_user_cmd(char * user_input, char path_dirs[12][144], char * env[]);
// void eval_user_cmd(char * user_input, char path_dirs[12][144], char * env[]);

// dynamic memory replacement
char mbuf1[64] = "", mbuf2[64] = "", mbuf3[64] = "", mbuf4[64] = "", 
	mbuf5[64] = "", mbuf6[64] = "", mbuf7[64] = "", mbuf8[64] = "";
char *mbufs[8] = { mbuf1, mbuf2, mbuf3, mbuf4, mbuf5, mbuf6, mbuf7, mbuf8 };
int mindex = 0;

// drives rnsh program
//
int main(int argc, char * argv[]) {
    // char ** env_ptr = env;

    // /*while (*env_ptr != NULL) { // for debugging/reference
    //     printf("%s\n", *env_ptr);
    //     env_ptr++;
    // }
    // putchar('\n');
    // env_ptr = env;*/
    
    // // get HOME and PATH vars from env
    // char env_HOME[144] = "";
    // char env_PATH[144] = "";

    // while (*env_HOME == '\0' || *env_PATH == '\0') {
    //     if (strncmp(*env_ptr, "HOME", 4) == 0) {
    //         strcpy(env_HOME, *env_ptr + 5);
    //     }
    //     else if (strncmp(*env_ptr, "PATH", 4) == 0) {
    //         strcpy(env_PATH, *env_ptr + 5);
    //     }
    //     env_ptr++;
    // }

    // // for debugging/reference
    // /*printf("$HOME: %s\n", env_HOME);
    // printf("$PATH: %s", env_PATH);
    // putchar('\n');*/

    // // tokenize PATH var
    // char path_dirs[12][144] = {{'\0'}};
    // int i = 0;
    // char * curr_dir = strtok(env_PATH, ":");

    // while (curr_dir != NULL) {
    //     strcpy(path_dirs[i++], curr_dir);
    //     curr_dir = strtok(NULL, ":");
    // }
    // //path_dirs[i][0] = '\0';

    // begin execution loop
    char user_input[144] = "";

    do {
        printf("rmn_sh $ ");
        //fgets(user_input, 143, stdin);
        //user_input[strlen(user_input) - 1] = '\0'; // replace \n
		gets(user_input);

		if (strcmp(user_input, "logout") == 0) {
			exit(1);
		}
        
        if (strncmp(user_input, "cd", 2) == 0 && 
            (user_input[2] == '\0' || user_input[2] == ' ')) {
            
            if (user_input[2] == ' ') {
                char * new_dir = strtok(user_input, " ");
                new_dir = strtok(NULL, " ");
                chdir(new_dir);
            }
            else {
				chdir("/");
            }
        }
        else if (strncmp(user_input, "exit", 4) == 0) {
            exit(1);
        }
        else { // non-simple commands, will try to execute
			eval_user_cmd(user_input);
        }
    } while (1);

    return 0;
}

// frees memory and sets i, i + 1 args to NULL
void clear_user_args(char * user_args[], int i) {
    //free(user_args[i]); // memory leak but no way to resolve without free
	// alternative below
	memset(user_args[i], 0, 64);
    user_args[i] = NULL;

    //free(user_args[i + 1]);
	memset(user_args[i + 1], 0, 64);
    user_args[i + 1] = NULL;
}

// handles I/O redirection and will issue the call to execve() if cmd is valid
void exec_user_cmd(char * user_input) {
    // tokenize user args
    char * user_args[24] = { NULL };
	char user_input_copy[256];
	strcpy(user_input_copy, user_input);
    char * curr_arg = strtok(user_input, " ");
    int i = 0;

    while (curr_arg != NULL) {
        //user_args[i] = (char *)malloc(sizeof(char) * strlen(curr_arg) + 1);
		user_args[i] = mbufs[mindex++];
		if (mindex >= 8)
			printf("  sh: WARNING! Out of dynamic memory replacement!\n");
        strcpy(user_args[i++], curr_arg);
        curr_arg = strtok(NULL, " ");
    }

    // check for file redirection
    for(--i; i >= 0; i--) {
        if (strcmp(user_args[i], "<") == 0) { // redirect input
            close(0); // close stdin to keyboard
			// printf("  sh: < redirect, file == %s\n", user_args[i + 1]);

			// need to clear early for cat, etc.
			strcpy(user_args[i], user_args[i + 1]);
			memset(user_args[i + 1], 0, 64);
			user_args[i + 1] = NULL;
			mindex--;

            open(user_args[i], O_RDONLY);

            memset(user_args[i], 0, 64);
			user_args[i] = NULL;
			mindex--;

			//clear_user_args(user_args, i);
        }
        else if (strcmp(user_args[i], ">") == 0) {
            close(1); // close stdout to console

			strcpy(user_args[i], user_args[i + 1]);
			memset(user_args[i + 1], 0, 64);
			user_args[i + 1] = NULL;
			mindex--;
			
            open(user_args[i], O_WRONLY | O_CREAT);

			memset(user_args[i], 0, 64);
			user_args[i] = NULL;
			mindex--;
        }
        else if (strcmp(user_args[i], ">>") == 0) {
            close(1);
            // when calling open(), MUST specify one of O_RDONLY, O_WRONLY, or O_RDWR
            // O_APPEND is an additional option to be bitwise OR'ed

			strcpy(user_args[i], user_args[i + 1]);
			memset(user_args[i + 1], 0, 64);
			user_args[i + 1] = NULL;
			mindex--;

            open(user_args[i], O_RDWR | O_APPEND | O_CREAT);

            memset(user_args[i], 0, 64);
			user_args[i] = NULL;
			mindex--;
        }
    }

	strcpy(user_input_copy, user_args[0]);
	for (i = 1; i < 8 && user_args[i] != NULL; i++) {
		strcat(user_input_copy, " ");
		strcat(user_input_copy, user_args[i]);
	}

	// printf("  sh: user_input_copy == %s\n", user_input_copy);
	// printf("  sh: uic strlen = %d\n",strlen(user_input_copy));

	exec(user_input_copy);

    exit(100);
}

// recursively checks for pipes and passes along execution appropriately
void eval_pipe_cmd(char * user_input) {
    // check for pipes
    int i = 0;
    for ( ; user_input[i] != '\0' && user_input[i] != '|'; i++) {
        // finds either first pipe or null char
    }
    if (user_input[i] == '\0') { // no pipe, execute normally
		exec_user_cmd(user_input);
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
        //exec_user_cmd(user_input, path_dirs, env);
		exec_user_cmd(user_input);

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
        //eval_pipe_cmd(tail_cmd, path_dirs, env);
		eval_pipe_cmd(tail_cmd);
    }
}

// forks process and passes along execution
void eval_user_cmd(char * user_input) {
    int pid = fork(); // returns child pid if parent, 0 if child
    
    if (pid < 0) { // no more PROCs in kernel
        //perror("fork failed");
		printf("  sh: fork failed!\n");
        exit(1);
    }
    else if (pid > 0) { // parent process
        int status = 0;
        pid = wait(&status); // child's exit code will be stored in status
        printf("\n  child process %d terminated with code %d\n", pid, status);
    }
    else { // child process
        // check for pipes and continue execution
		eval_pipe_cmd(user_input);
    }
}