// Code based on/started from code written for Lab6 PreWork, dir.c

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <ext2fs/ext2_fs.h>

#define BLKSIZE 1024

// define shorter TYPES, save typing efforts
typedef struct ext2_group_desc GD;
typedef struct ext2_super_block SUPER;
typedef struct ext2_inode INODE;
typedef struct ext2_dir_entry_2 DIR; // need this for new version of e2fs

GD *gp;
SUPER *sp;
INODE *ip;
DIR *dp;

int fd;
int iblock;

int get_block(int fd, int blk, char buf[])
{
    lseek(fd, (long)blk * BLKSIZE, 0);
    read(fd, buf, BLKSIZE);
}

int dir()
{
    char dbuf[BLKSIZE];

    // First, read the block into a char dbuf[1024].

    // read GD
    get_block(fd, 2, dbuf);
    gp = (GD *)dbuf;

    iblock = gp->bg_inode_table; // get inode start block#
    printf("inode_block=%d\n", iblock);

    // get inode start block
    get_block(fd, iblock, dbuf);

    // ip = (INODE *)buf; would make ip point at the /first/ inode
    // root is found at second inode however
    ip = (INODE *)dbuf + 1; // point ip to the second inode, which points to root

    // Important! Remember that i_block is an array of disk/data block numbers!
    // Indices 0-11 (the first 12 blocks) are direct blocks, or the actual 
    //   disk block numbers. Indices 12, 13, and 14 point to blocks of more 
    //   block numbers, with single, double, and triple indirection respectively

    // Let DIR *dp and char *cp BOTH point at dbuf;
    // Use dp-> to access the fields of the record, e.g. print its name

    DIR * dp;
    char * cp;
    int i = 0, j = 0;

    for (i = 0; i < 12; i++) { // for now assume only direct blocks
        if (ip->i_block[i] == 0) {
            // if this is 0, data not found or no more data
            break;
        }
        dp = (DIR *)dbuf;
        cp = dbuf;
        int debug = ip->i_block[i];
        printf("data block number=%d\n", ip->i_block[i]);
        get_block(fd, ip->i_block[i], dbuf); // get the inode's /data/ block
        //ip->i_block[i] gets set to 0 after?? why???
        int debug2 = ip->i_block[i];
        //printf("data block number=%d\n", ip->i_block[i]);
        printf("inode       rec_len     name_len    file_type   name\n");

        while (cp < dbuf + BLKSIZE) { // while still within the data block
            printf("%8d    ", dp->inode);
            printf("%8d    ", dp->rec_len);
            printf("%8d    ", dp->name_len);
            printf("%8d    ", dp->file_type);
            for (j = 0; j < dp->name_len; j++) {
                printf("%c", dp->name[j]);
            }
            putchar('\n');

            cp += dp->rec_len;
            dp = (DIR *)cp;
        }
    }
}

int search(INODE * dir_inode, char * name)
{
    char dbuf[BLKSIZE];

    get_block(fd, dir_inode->i_block[0], dbuf); // get the inode's /data/ block

    // Let DIR *dp and char *cp BOTH point at dbuf;
    // Use dp-> to access the fields of the record, e.g. print its name

    DIR * dp;
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

        printf("data block number=%d\n", ip->i_block[i]);

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

void showblock(char * diskname, char * pathname)
{
    fd = open(diskname, O_RDONLY); // this serves as the dev no
    if (fd < 0)
    {
        printf("open %s failed\n", diskname);
        exit(1);
    }

    char pathcopy[256] = ""; // assumed to be absolute path
    strcpy(pathcopy, pathname);
    int n = 0;

    char * filenames[24] = { NULL };
    filenames[0] = strtok(pathcopy, "/");

    // tokenize the rest of the filenames
    for (int i = 1; i < 24 && filenames[i - 1] != NULL; i++, n++) {
        filenames[i] = strtok(NULL, "/");
    }

    // start searching from the root inode

    char dbuf[BLKSIZE];

    // read GD block
    get_block(fd, 2, dbuf);
    gp = (GD *)dbuf;

    iblock = gp->bg_inode_table; // get inode start block num
    printf("inode_block = %d\n", iblock);

    // get inode start block
    get_block(fd, iblock, dbuf);

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
        ino = search(ip, filenames[i]);
        if (ino == 0) {
            printf("%s not found\n", filenames[i]);
            exit(2);
        }

        // Use Mailman's Algorithm to convert (dev ==> fd here, ino) to inode ptr
        dblock = (ino - 1) / 8 + iblock; // data/disk block containing this inode
        offset = (ino - 1) % 8; // offset of the inode in this block
        get_block(fd, dblock, ibuf);
        ip = (INODE *) ibuf + offset;
    }

    // now ip points at the inode of the file at the end of the pathname

    printf("\n    direct block numbers\n");
    for (i = 0; i < 12 && ip->i_block[i] != 0; i++) {
        printf("%4d ", ip->i_block[i]);
    }
    printf("\n");

    // NOTE!!! Must use separate buffers from ibuf!!!

    if (ip->i_block[12] != 0) { // look for single indirect blocks
        printf("\n    indirect block numbers, ip->i_block[12] = %d\n", ip->i_block[12]);
        char ind_buf[BLKSIZE] = "";
        int *track = &(ip->i_block[13]);

        get_block(fd, ip->i_block[12], ind_buf);
        int * blockptr = (int *) ind_buf;

        for (i = 0; i < 256 && blockptr[i] != 0; i++) {
            printf("%4d ", blockptr[i]);

            if ((i + 1) % 16  == 0) {
                putchar('\n');
            }
        }
        printf("\n");
    }

    if (ip->i_block[13] != 0) { // look for double indirect blocks
        printf("    double indirect block numbers, ip->i_block[13] = %d\n\n", ip->i_block[13]);
        
        char ind_buf[BLKSIZE] = "", dbl_buf[BLKSIZE] = "";
        get_block(fd, ip->i_block[13], ind_buf);
        int * blockptr = (int *) ind_buf, * dbl_blk_ptr = NULL, j = 0;

        for (i = 0; i < 1 && blockptr[i] != 0; i++) {
            printf("  blockptr[i] = %d\n", blockptr[i]);
            
            get_block(fd, blockptr[i], dbl_buf);
            dbl_blk_ptr = (int *) dbl_buf;

            for (j = 0; j < 256 && dbl_blk_ptr[j] != 0; j++) {
                printf("%4d ", dbl_blk_ptr[j]);

                if ((j + 1) % 16 == 0) {
                    putchar('\n');
                }
            }
            printf("\n");
        }
    }
}

int main(int argc, char * argv[]) {
    showblock(argv[1], argv[2]);

    return 0;
}