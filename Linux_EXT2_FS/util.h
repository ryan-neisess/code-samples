#ifndef UTIL
#define UTIL

#include "type.h"

/*************** globals for Level-1 ********************/

// Externally declared globals for other .c files
// All defined in main.c

extern MINODE minode[NMINODE];
extern MINODE *root;
extern PROC   proc[NPROC], *running;

extern char gpath[256];
extern char *name[64]; // assume at most 64 components in pathnames
extern int  n;

extern int  fd, dev;
extern int  nblocks, ninodes, bmap, imap, inode_start;
extern char line[256], cmd[32], pathname[256];

#endif

// General purpose functions for ext2fs

void rec_abs_path(MINODE * wd, char abs_path[]);
void make_abs_path(char rel_path[]);

int my_get_block(int fd, int blk, char buf[]);
int my_put_block(int fd, int blk, char buf[]);
MINODE * my_iget(int dev, int ino);
int my_iput(MINODE *mip);
int my_search(INODE * dir_inode, char name[], int fd);
int my_getino(int * fd, char local_pathname[]);