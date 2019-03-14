#include <stdio.h>
#include <stdlib.h>
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

        //printf("inode       rec_len     name_len    file_type   name\n");

        while (cp < dbuf + BLKSIZE) { // while still within the data block
            for (j = 0; j < dp->name_len; j++) {
                temp[j] = dp->name[j];
            }
            temp[j] = '\0';
            if (strcmp(temp, name) == 0) { // match found
                return dp->inode;
            }

            cp += dp->rec_len;
            dp = (DIR *)cp;
        }
    }

    return 0;
}

char *disk = "mydisk";

int main(int argc, char *argv[])
{
    if (argc > 1)
        disk = argv[1];

    fd = open(disk, O_RDONLY);
    if (fd < 0)
    {
        printf("open %s failed\n", disk);
        exit(1);
    }

    dir();
}
