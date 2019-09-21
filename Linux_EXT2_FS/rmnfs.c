//#include <term.h> // trying for bolding font

#include "util.c"
#include "mount_root.c"
#include "cd_ls_pwd.c"
#include "mkdir_creat_rmdir.c"
#include "link_unlink_symlink.c"
#include "lv1_misc.c"
#include "open_close_lseek.c"
#include "read_cat.c"
#include "write_cp_mv.c"

/*************** globals for Level-1 ********************/

MINODE minode[NMINODE];
MINODE *root;
PROC   proc[NPROC], *running;

char gpath[256];
char *name[64]; // assume at most 64 components in pathnames
int  n;

int  fd, dev;
int  nblocks, ninodes, bmap, imap, inode_start;
char line[256], cmd[32], pathname[256], * args[12] = { NULL };

int quit(void); // needs to be declared before fptrs

char * cmd_list[] = { "cd", "ls", "pwd", "mkdir", "creat", "rmdir", 
    "link", "unlink", "symlink", "chmod", "pfd", "open", "close", "lseek", 
    "read", "cat", "write", "cp", "quit", NULL };
int (* fptrs[])() = { (int (*) () )cd, ls, pwd, make_dir, create_file, rm_dir, 
    rmn_link, un_link, rmn_symlink, ch_mod, pfd, rmn_open, rmn_close, rmn_lseek, 
    read_file, rmn_cat, rmn_write, rmn_cp, quit };

int identify_cmd(void);

int main(int argc, char * argv[]) {
    init();
    mount_root(argv[1]);
    //setupterm(NULL, STDOUT_FILENO, NULL);

    char * temp = NULL;
    int cmd_num = -1, result = 0, i =0;
    while (1) {
        //putp(enter_bold_mode);
        printf("rmnfs:");
        printf("$ ");
        //putp(exit_attribute_mode);
        fgets(line, 256, stdin);
        line[strlen(line) - 1] = '\0';
        temp = strtok(line, " "); // temp should point to beginning
        temp = strtok(NULL, " "); // temp points to pathname if exists, NULL o.w.
        strcpy(cmd, line);
        if (temp == NULL) {
            strcpy(pathname, "");
        }
        else{
            strcpy(pathname, temp);
        }
        // above kept to prevent need for immediate refactoring of code
        // get further tokens below
        args[0] = line;
        args[1] = temp;
        i = 2;
        while (temp = strtok(NULL, " ")) {
            args[i++] = temp;
        }

        cmd_num = identify_cmd();
        if (cmd_num == -1) {
            printf("command not recognized\n");
        }
        else {
            result = fptrs[cmd_num]();
        }
    }

    return 0;
}

int identify_cmd(void) {
    int i = 0;
    for (i = 0; cmd_list[i]; i++) {
        if (!strcmp(cmd, cmd_list[i])) {
            return i;
        }
    }
    return -1; // if command isn't found in list
}

int quit(void) {
    int i = 0;
    for (i = 0; i < NMINODE; i++) {
        if (minode[i].dirty && minode[i].refCount > 0) {
            iput(&minode[i]);
        }
    }

    exit(0);
    return 24;
}