//#include "util.h"

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
extern char line[256], cmd[32], pathname[256], * args[12];

// General purpose functions for ext2fs

int rec_abs_path(MINODE * wd, char abs_path[]);
void make_abs_path(char rel_path[]);

int get_block(int fd, int blk, char buf[]);
int put_block(int fd, int blk, char buf[]);
MINODE * iget(int dev, int ino);
int iput(MINODE *mip);
int search(INODE * dir_inode, char name[], int fd);
int getino(int * fd, char local_pathname[]);

// Uses logic based on rpwd()
int rec_abs_path(MINODE * wd, char abs_path[]) {
    if (wd == root){
        return 1;
    }
    char buf[BLKSIZE], curr_name[256] = "";
    get_block(dev, wd->INODE.i_block[0], buf);

    dp = (DIR *)buf; // dp points to info for .
    int curr_inode = dp->inode;

    dp = (DIR *)(buf + dp->rec_len); // dp points to info for ..
    int parent_inode = dp->inode;
    MINODE * pip = (MINODE *)iget(dev, parent_inode);

    // search through parent directory for matching curr_inode
    int i = 0, found = 0;
    char * cp = NULL;
    for (i = 0; i < 12 && found == 0; i++) {
        get_block(dev, pip->INODE.i_block[i], buf);
        cp = buf;
        dp = (DIR *)buf;

        while (cp < buf + BLKSIZE && found == 0) {
            if (curr_inode == dp->inode) { // match found
                found = 1;
            }
            else {
                cp += dp->rec_len;
                dp = (DIR *)cp;
            }
        }
    }

    if (found == 0) {
        printf("  rec_abs_path(): error: ino %d not found in parent ino %d\n", 
            wd->ino, pip->ino);
        return 0;
    }

    // dp should now be pointing at the correct record; save the name
    strncpy(curr_name, dp->name, dp->name_len);
    curr_name[dp->name_len]= '\0';

    int result = rec_abs_path(pip, abs_path);

    strcat(abs_path, "/");
    strcat(abs_path, curr_name);

    return result;
}

void make_abs_path(char rel_path[]) {
    if (rel_path[0] == '/') {
        return;
    }
    else if (running->cwd == root) {
        char temp_path[256] = "";
        strcpy(temp_path, rel_path);
        rel_path[0] = '/';
        strcpy(rel_path + 1, temp_path);
    }
    else {
        char temp_path[256] = "";
        if (!rec_abs_path(running->cwd, temp_path)) {
            printf("  make_abs_path(): error: path %s invalid\n", rel_path);
        }
        if (rel_path[0] != '\0') {
            strcat(temp_path, "/");
            strcat(temp_path, rel_path);
        }
        strcpy(rel_path, temp_path);
        printf("  make_abs_path(): abs path: %s\n", rel_path);
    }
}

int get_block(int fd, int blk, char buf[]) {
    lseek(fd, (long)blk * BLKSIZE, 0);
    read(fd, buf, BLKSIZE);
}

int put_block(int fd, int blk, char buf[]) {
    lseek(fd, (long)blk * BLKSIZE, 0);
    write(fd, buf, BLKSIZE);
}

MINODE * iget(int dev, int ino) {
    //   return minode pointer to loaded INODE
    //   (1). Search minode[ ] for an existing entry (refCount > 0) with 
    //        the needed (dev, ino):
    //        if found: inc its refCount by 1;
    //                  return pointer to this minode;
    int i = 0;
    for (i = 0; i < NMINODE; i++) {
        if (minode[i].refCount > 0 && //check this first for short circuit eval
            minode[i].dev == dev && minode[i].ino == ino) {
            // dev and ino pairwise match found w/ positive refCount

            minode[i].refCount++;
            return &minode[i];
        }
    }

    //   (2). // needed entry not in memory:
    //        find a FREE minode (refCount = 0); Let mip-> to this minode;
    //        set its refCount = 1;
    //        set its dev, ino
    MINODE * mip = NULL;

    for (i = 0; i < NMINODE; i++) {
        if (minode[i].refCount == 0) {
            minode[i].refCount = 1;
            minode[i].dev = dev;
            minode[i].ino = ino;
            mip = &minode[i];
            break;
        }
    }

    //   (3). load INODE of (dev, ino) into mip->INODE:

    // get INODE of ino a char buf[BLKSIZE]
    char buf[BLKSIZE];
    int blk = 0, offset = 0;
    blk = (ino - 1) / 8 + inode_start;
    offset = (ino - 1) % 8;

    printf("  iget(): ino = %d, blk = %d, offset = %d\n", ino, blk, offset);

    get_block(dev, blk, buf);
    ip = (INODE *)buf + offset;
    mip->INODE = *ip; // copy INODE to mp->INODE

    return mip;
}

int iput(MINODE *mip) { // dispose a used minode by mip
    mip->refCount--;

    if (mip->refCount > 0)
        return 0;
    if (!mip->dirty)
        return 0;

    // Write YOUR CODE to write mip->INODE back to disk
    // Alg based on opposite of iget()'s (3).
    char buf[BLKSIZE];
    int blk = 0, offset = 0;
    blk = (mip->ino - 1) / 8 + inode_start;
    offset = (mip->ino - 1) % 8;

    get_block(dev, blk, buf);
    ip = (INODE *)buf + offset;
    *ip = mip->INODE;
    put_block(dev, blk, buf);

    return 1;
}

int search(INODE * dir_inode, char name[], int fd) {
    char dbuf[BLKSIZE];

    get_block(fd, dir_inode->i_block[0], dbuf); // get the inode's /data/ block

    // Let DIR *dp and char *cp BOTH point at dbuf;
    // Use dp-> to access the fields of the record, e.g. print its name

    char * cp, temp[256] = "";
    int i = 0, j = 0;

    for (i = 0; i < 12; i++) { // for now assume only direct blocks
        if (ip->i_block[i] == 0) {
            // if this is 0, data not found or no more data
            break;
        }
        dp = (DIR *)dbuf;
        cp = dbuf;
        int debug = ip->i_block[i];

        printf("  search(): data block number = %d\n", ip->i_block[i]);

        get_block(fd, ip->i_block[i], dbuf); // get the inode's /data/ block

        int debug2 = ip->i_block[i];

        printf("inode       rec_len     name_len    file_type   name\n");

        while (cp < dbuf + BLKSIZE) { // while still within the data block
            strncpy(temp, dp->name, dp->name_len);
            temp[dp->name_len] = '\0'; // NOTE: strncpy() does not guarantee '\0' append

            printf("%8d    ", dp->inode);
            printf("%8d    ", dp->rec_len);
            printf("%8d    ", dp->name_len);
            printf("%8d    ", dp->file_type);
            printf("%s\n", temp);

            if (strcmp(temp, name) == 0) { // match found
                return dp->inode;
            }

            cp += dp->rec_len;
            dp = (DIR *)cp;
        }
    }

    return 0;
}

int getino(int * fd, char local_pathname[]) {
    // catch edge case for root
    if (strcmp(local_pathname, "/") == 0) {
        return root->ino;
    }
    char pathcopy[256] = "";
    strcpy(pathcopy, local_pathname);
    if (pathcopy[0] != '/') {
        make_abs_path(pathcopy);
    }
    int n = 0;

    char * filenames[24] = { NULL };
    filenames[0] = strtok(pathcopy, "/");

    // tokenize the rest of the filenames
    for (int i = 1; i < 24 && filenames[i - 1] != NULL; i++, n++) {
        filenames[i] = strtok(NULL, "/");
    }

    // start searching from the root inode

    char dbuf[BLKSIZE];

    // get inode start block
    get_block(*fd, inode_start, dbuf); // NOTE! Change needed here for KC specs

    // ip = (INODE *)buf; would make ip point at the /first/ inode
    // root is found at second inode however
    ip = (INODE *)dbuf + 1; // point ip to the second inode, which points to root

    // Important! Remember that i_block is an array of disk/data block numbers!
    // Indices 0-11 (the first 12 blocks) are direct blocks, or the actual 
    //   disk block numbers. Indices 12, 13, and 14 point to blocks of more 
    //   block numbers, with single, double, and triple indirection respectively

    int ino = -1, dblock = -1, offset = -1, i = 0;
    char ibuf[BLKSIZE] = "";

    for (i = 0; i < n; i++) {
        ino = search(ip, filenames[i], *fd);
        if (ino == 0) {
            printf("  getino(): %s not found\n", filenames[i]);
            return -4;
        }

        // Use Mailman's Algorithm to convert (dev ==> fd here, ino) to inode ptr
        dblock = (ino - 1) / 8 + inode_start; // data/disk block containing this inode
        offset = (ino - 1) % 8; // offset of the inode in this block
        get_block(*fd, dblock, ibuf);
        ip = (INODE *) ibuf + offset;
    }

    // now ip points at the inode of the file at the end of the pathname
    // ino also is the inode number of that file

    //printf("  getino(): inode num = %d\n", ino);
    return ino;
}

#endif