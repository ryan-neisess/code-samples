#include "util.c"

extern char * read_link(char link_name[]);

char * t1 = "xwrxwrxwr-------";

int cd(void);
void ls_file(int ino, char filename[]);
int ls(void);
int rpwd(MINODE * wd);
int pwd(void);

// Precondition: if no pathname specified, pathname is empty string
//   This is ensured in shell-like part of program
int cd(void) {
    if (pathname[0] == '\0') { // default to root
        strcpy(pathname, "/");
    }
    else if (pathname[0] != '/') {
        make_abs_path(pathname); // make the given pathname absolute
    }
    // (1).  ino = getino(pathname);
    int ino = getino(&dev, pathname);

    if (ino < 0) {
        return ino; // will return error code
    }

    // (2).  mip = iget(dev, ino);
    MINODE * mip = (MINODE *)iget(dev, ino);

    // (3).  verify mip->INODE is a DIR
    if (!S_ISDIR(mip->INODE.i_mode)) {
        printf("  cd(): error: %s is not a directory\n", pathname);
        return -1; // exit(1); instead?
    }

    // (4).  iput(running->cwd);
    iput(running->cwd);

    // (5).  running->cwd = mip;
    running->cwd = mip;

    // rmn extras
    mip->INODE.i_atime = time(0L);
    mip->dirty = 1;

    return 1;
}

void ls_file(int ino, char filename[]) {
    MINODE * mip = (MINODE *)iget(dev, ino);

    // print file info
    if ((mip->INODE.i_mode & 0xF000) == 0x8000) // if (S_ISREG())
        printf("-");
    if ((mip->INODE.i_mode & 0xF000) == 0x4000) // if (S_ISDIR())
        printf("d");
    if ((mip->INODE.i_mode & 0xF000) == 0xA000) // if (S_ISLNK())
        printf("l");

    int i = 0;
    for (i = 8; i >= 0; i--) {
        if (mip->INODE.i_mode & (1 << i)) // print r|w|x
            printf("%c", t1[i]);
        else // or print -
            printf("-");
    }
    printf("  %4d  ", mip->INODE.i_links_count);
    printf("%4d  ", mip->INODE.i_gid);
    printf("%4d  ", mip->INODE.i_uid);
    printf("%8d  ", mip->INODE.i_size);
    // print time
    char ftime[64] = "";
    strcpy(ftime, ctime((const time_t *)&mip->INODE.i_mtime));
    ftime[strlen(ftime) - 1] = '\0'; // remove \n at end
    printf("%s  ", ftime);
    // print file name
    printf("%s", filename);

    // * * * TEST THIS!!! * * *

    if ((mip->INODE.i_mode & 0xF000) == 0xA000){
        /* // NOTE: Linux's readlink() works DIFFERENT from KC's!!!
        char buf[BLKSIZE];
        int bytes_copied = readlink(pathname, buf, BLKSIZE);
        printf(" -> %s", buf); 
        */
        //printf(" -> %s", read_link(filename)); 
        printf(" -> %s", (char *)mip->INODE.i_block); 
    }
    putchar('\n');
    iput(mip);  
}

int ls(void) {
    if (pathname[0] == '\0') {
        // this will build an abs path from the cwd
        make_abs_path(pathname);
    }
    else if (pathname[0] != '/') {
        make_abs_path(pathname); // make the given pathname absolute
    }
    int ino = getino(&dev, pathname);
    if (ino < 0) {
        return ino; // will return error code
    }
    MINODE * mip = (MINODE *)iget(dev, ino);

    if (!S_ISDIR(mip->INODE.i_mode)) {
        printf("  ls(): error: %s is not a directory\n", pathname);
        iput(mip);
        return -2;
    }

    char buf[BLKSIZE], filename[256] = "";
    get_block(dev, mip->INODE.i_block[0], buf);
    dp = (DIR *)buf;
    char * cp = buf;;

    printf("f  prmssns links owner group      size             last_modified  filename\n");

    while (cp < buf + BLKSIZE) {
        strncpy(filename, dp->name, dp->name_len);
        filename[dp->name_len] = '\0';
        ls_file(dp->inode, filename); // pass in so links format properly

        cp += dp->rec_len;
        dp = (DIR *)cp;
    }
    iput(mip);
}

int rpwd(MINODE * wd) {
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

    // Notes
    // Assuming a test run with /dir1 created first and cwd:
    // Here, parent_inode is the inode number of root (2)
    // curr_inode meanwhile is the inode number of the cwd/of dir1 (12)
    // We need the data block for parent directory (root here) in order to 
    //   look through the entires in the directory, hence pip->INODE.i_block[0]
    get_block(dev, pip->INODE.i_block[0], buf);
    char * cp = buf;
    dp = (DIR *)buf;

    while (cp < buf + BLKSIZE) {
        if (curr_inode == dp->inode) { // match found
            break;
        }
        cp += dp->rec_len;
        dp = (DIR *)cp;
    }

    // dp should now be pointing at the correct record; save the name
    strncpy(curr_name, dp->name, dp->name_len);
    curr_name[dp->name_len]= '\0';

    int result = rpwd(pip);

    printf("/%s", curr_name);

    iput(pip);

    return result;
}

int pwd(void) {
    if (running->cwd == root) {
        printf("/\n");
        return 1;
    }
    int result = rpwd(running->cwd);
    putchar('\n');
    return result;
}