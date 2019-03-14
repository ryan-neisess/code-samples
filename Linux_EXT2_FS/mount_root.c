#include "util.c"

void init(void);
void mount_root(char diskname[]);

void init(void) {
    // (1). 2 PROCs, P0 with uid=0, P1 with uid=1, all PROC.cwd = 0
    int i = 0;
    for (i = 0; i < NPROC; i++) {
        proc[i].cwd = NULL;
    }
    proc[0].uid = 0;
    proc[1].uid = 1;

    // (2). MINODE minode[64]; all with refCount=0
    for (i = 0; i < NMINODE; i++) {
        minode[i].refCount = 0;
        minode[i].dirty = 0;
    }

    // (3). MINODE *root = 0;
    root = NULL;
}

void mount_root(char diskname[]) {
    // open device for RW (get a file descriptor as dev for the opened device)
    dev = open(diskname, O_RDWR);
    if (dev < 0) {
        printf("  failed to open disk\n");
        exit(1);
    }

    // read SUPER block to verify it's an EXT2 FS

    char buf[BLKSIZE];
    get_block(dev, 1, buf); // read SUPER block
    sp = (SUPER *)buf;

    printf("s_magic = %x\n", sp->s_magic); 
    if (sp->s_magic != 0xEF53) // check for EXT2 magic number
    {
        printf("NOT an EXT2 FS\n");
        exit(1);
    }
    printf("EXT2 FS OK\n");

    // record nblocks, ninodes as globals

    nblocks = sp->s_blocks_count;
    ninodes = sp->s_inodes_count;

    // read GD0; record bamp, imap, inodes_start as globals;

    get_block(dev, 2, buf);
    gp = (GD *)buf;
    bmap = gp->bg_block_bitmap;
    imap = gp->bg_inode_bitmap;
    inode_start = gp->bg_inode_table;

    root = (MINODE *)iget(dev, 2);    /* get root inode */

    // Let cwd of both P0 and P1 point at the root minode (refCount=3)

    proc[0].cwd = (MINODE *)iget(dev, 2);
    proc[1].cwd = (MINODE *)iget(dev, 2);

    // initialze fd[] for both to NULL

    int i = 0;
    for (i = 0; i < NFD; i++) {
        proc[0].fd[i] = NULL;
        proc[1].fd[i] = NULL;
    }

    // Let running -> P0

    running = &proc[0];
}