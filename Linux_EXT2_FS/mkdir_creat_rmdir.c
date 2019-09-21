#include "util.c"

int tst_bit(char *buf, int bit);
int set_bit(char *buf, int bit);
int clr_bit(char *buf, int bit);

int decFreeInodes(int dev);
int ialloc(int dev);
int idealloc(int dev, int ino);

int decFreeBlocks(int dev);
int balloc(int dev);
int bdealloc(int dev, int ino);

int enter_dir_name(MINODE * pip, int new_ino, char * new_name);
int my_mkdir(MINODE * pip, char * name);
int make_dir(void);

int enter_reg_name(MINODE * pip, int new_ino, char * new_name);
int my_creat(MINODE * pip, char * name);
int create_file(void);

int rm_child_name(MINODE * pip, char * old_name);
int rm_dir(void);

int tst_bit(char *buf, int bit) {
    int i, j;
    i = bit / 8;
    j = bit % 8;
    if (buf[i] & (1 << j))
        return 1;
    return 0;
}

int set_bit(char *buf, int bit) {
    int i, j;
    i = bit / 8;
    j = bit % 8;
    buf[i] |= (1 << j);
}

int clr_bit(char *buf, int bit) {
    int i, j;
    i = bit / 8;
    j = bit % 8;
    buf[i] &= ~(1 << j);
}

int decFreeInodes(int dev) {
    char buf[BLKSIZE];

    // dec free inodes count in SUPER and GD
    get_block(dev, 1, buf);
    sp = (SUPER *)buf;
    sp->s_free_inodes_count--;
    put_block(dev, 1, buf);

    get_block(dev, 2, buf);
    gp = (GD *)buf;
    gp->bg_free_inodes_count--;
    put_block(dev, 2, buf);
}

int ialloc(int dev) {
    int i;
    char buf[BLKSIZE];

    // read inode_bitmap block
    get_block(dev, imap, buf);

    for (i = 0; i < ninodes; i++)
    {
        if (tst_bit(buf, i) == 0)
        {
            set_bit(buf, i);
            decFreeInodes(dev);

            put_block(dev, imap, buf);

            return i + 1;
        }
    }
    printf("ialloc(): no more free inodes\n");
    return 0;
}

int idealloc(int dev, int ino) {
    int i = 0;
    char buf[BLKSIZE];
    if (ino > ninodes) {
        printf("  idealloc(): error: ino out of range (high)\n");
        return -1;
    }

    get_block(dev, imap, buf); // get imap block
    clr_bit(buf, ino - 1);
    put_block(dev, imap, buf); // write buf back

    // incr free inodes count in SUPER and GD
    get_block(dev, 1, buf);
    sp = (SUPER *)buf;
    sp->s_free_inodes_count++;
    put_block(dev, 1, buf);

    get_block(dev, 2, buf);
    gp = (GD *)buf;
    gp->bg_free_inodes_count++;
    put_block(dev, 2, buf);
}

int decFreeBlocks(int dev) {
    char buf[BLKSIZE];

    // dec free inodes count in SUPER and GD
    get_block(dev, 1, buf);
    sp = (SUPER *)buf;
    sp->s_free_blocks_count--;
    put_block(dev, 1, buf);

    get_block(dev, 2, buf);
    gp = (GD *)buf;
    gp->bg_free_blocks_count--;
    put_block(dev, 2, buf);
}

int balloc(int dev) {
    int i;
    char buf[BLKSIZE];

    // read block_bitmap block
    get_block(dev, bmap, buf);

    for (i = 0; i < nblocks; i++)
    {
        if (tst_bit(buf, i) == 0)
        {
            set_bit(buf, i);
            decFreeBlocks(dev);

            put_block(dev, bmap, buf);

            return i + 1;
        }
    }
    printf("balloc(): no more free blocks\n");
    return 0;
}

int bdealloc(int dev, int bno) {
    int i = 0;
    char buf[BLKSIZE];
    if (bno > nblocks) {
        printf("  bdealloc(): error: bno out of range (high)\n");
        return -1;
    }

    get_block(dev, bmap, buf); // get imap block
    clr_bit(buf, bno - 1);
    put_block(dev, bmap, buf); // write buf back

    // incr free blocks count in SUPER and GD
    get_block(dev, 1, buf);
    sp = (SUPER *)buf;
    sp->s_free_blocks_count++;
    put_block(dev, 1, buf);

    get_block(dev, 2, buf);
    gp = (GD *)buf;
    gp->bg_free_blocks_count++;
    put_block(dev, 2, buf);
}

int enter_dir_name(MINODE * pip, int new_ino, char * new_name) {
    // 8. int enter_name(MINODE *pip, int myino, char *myname)
    // {
    // For each data block of parent DIR do { // assume: only 12 direct blocks

    int i = 0, ideal_len = 0, remaining = 0, new_name_len = strlen(new_name);
    int needed_len = 4 * ( (8 + new_name_len + 3) / 4 ), entry_entered = 0;
    char buf[BLKSIZE], * cp = NULL, temp = '\0';

    for (i = 0; i < 12; i++) {
        //     if (i_block[i]==0) BREAK;

        if (pip->INODE.i_block[i] == 0)
            break; // reached the end of currently allocated blocks

        // (1). get parent's data block into a buf[];
        
        // (2). EXT2 DIR entries: Each DIR entry has rec_len and name_len. Each entry's
        //     ideal length is   

        //         IDEAL_LEN = 4*[ (8 + name_len + 3)/4 ]

        //     All DIR entries in a data block have rec_len = IDEAL_LEN, except the last
        //     entry. The rec_len of the LAST entry is to the end of the block, which may
        //     be larger than its IDEAL_LEN.

        // --|-4---2----2--|----|---------|--------- rlen ->------------------------|
        //     |ino rlen nlen NAME|.........|ino rlen nlen|NAME                       |
        // --------------------------------------------------------------------------

        // (3). To enter a new entry of name with n_len, the needed length is

        //         need_length = 4*[ (8 + n_len + 3)/4 ]  // a multiple of 4

        // (4). Step to the last entry in a data block (HOW?).

        
        //     // get parent's ith data block into a buf[ ] 

        //     get_block(parent->dev, parent->INODE.i_block[i], buf);
        
        //     dp = (DIR *)buf;
        //     cp = buf;

        //     // step to LAST entry in block: int blk = parent->INODE.i_block[i];
            
        //     printf("step to LAST entry in data block %d\n", blk);
        //     while (cp + dp->rec_len < buf + BLKSIZE){

        //         /****** Technique for printing, compare, etc.******
        //          c = dp->name[dp->name_len];
        //         dp->name[dp->name_len] = 0;
        //         printf("%s ", dp->name);
        //         dp->name[dp->name_len] = c;
        //         **************************************************/

        //         cp += dp->rec_len;
        //         dp = (DIR *)cp;
        //     } 
        //     // dp NOW points at last entry in block

        get_block(pip->dev, pip->INODE.i_block[i], buf);
        dp = (DIR *)buf;
        cp = buf;

        printf("  enter_name(): stepping to last entry in block %d\n", 
            pip->INODE.i_block[i]);

        while (cp + dp->rec_len < buf + BLKSIZE) { // if ==, at the end, don't advance
            temp = dp->name[dp->name_len]; // save character
            dp->name[dp->name_len] = '\0'; // temporarily make name a string
            printf("    enter_name(): curr entry = %s\n",dp->name);
            dp->name[dp->name_len] = temp; // replace char with original

            cp += dp->rec_len;
            dp = (DIR *)cp;
        }
        // now dp points to the last entry in the current block

        //     Let remain = LAST entry's rec_len - its IDEAL_LENGTH;

        //     if (remain >= need_length){
        //         enter the new entry as the LAST entry and trim the previous entry
        //         to its IDEAL_LENGTH; 
        //         goto (6) below.
        //     } 

        ideal_len = 4 * ( (8 + dp->name_len + 3) / 4 );
        remaining = dp->rec_len - ideal_len;
        if (remaining >= needed_len) {
            // trim previous entry
            dp->rec_len = ideal_len;

            // advance to position for new entry
            cp += dp->rec_len;
            dp = (DIR *)cp;

            // write new entry to buf
            dp->inode = new_ino;
            dp->rec_len = buf + BLKSIZE - cp; // check this against remaining
            dp->name_len = new_name_len;
            strncpy(dp->name, new_name, new_name_len);
            dp->file_type = 2;

            entry_entered = 1;
            break;
        }

        //                             EXAMPLE:

        //                                 |LAST entry 
        // --|-4---2----2--|----|---------|--------- rlen ->------------------------|
        //     |ino rlen nlen NAME|.........|ino rlen nlen|NAME                       |
        // --------------------------------------------------------------------------
        //                                                     |     NEW entry
        // --|-4---2----2--|----|---------|----ideal_len-----|--- rlen=remain ------|
        //     |ino rlen nlen NAME|.........|ino rlen nlen|NAME|myino rlen nlen myname|
        // --------------------------------------------------------------------------

        // }

    }

    // (5).// Reach here means: NO space in existing data block(s)

    // Allocate a new data block; INC parent's isze by BLKSIZE;
    // Enter new entry as the first entry in the new data block with rec_len=BLKSIZE.

    // |-------------------- rlen = BLKSIZE -------------------------------------
    // |myino rlen nlen myname                                                  |
    // --------------------------------------------------------------------------

    if (!entry_entered) {
        int bno = balloc(pip->dev); // allocate a new disk block
        pip->INODE.i_size += BLKSIZE; // increment parent inode's size
        pip->INODE.i_block[i] = bno; // record block num in parent inode

        get_block(pip->dev, bno, buf); // get the corresponding (new) block from disk
        dp = (DIR *)buf;
        dp->inode = new_ino;
        dp->rec_len = BLKSIZE;
        dp->name_len = new_name_len;
        strncpy(dp->name, new_name, new_name_len);
        dp->file_type = 2;

        entry_entered = 1;
    }

    // (6).Write data block to disk;

    if (!entry_entered) {
        printf("  enter_dir_name(): error adding name to parent inode\n");
        return -6;
    }

    // rmn extras
    pip->INODE.i_ctime = pip->INODE.i_mtime = pip->INODE.i_atime = time(0L);
    pip->dirty = 1;

    put_block(pip->dev, pip->INODE.i_block[i], buf);

    return 1;
}

int my_mkdir(MINODE * pip, char * name) {
    // 1. pip points at the parent minode[] of "/a/b", name is a string "c") 

    // 2. allocate an inode and a disk block for the new directory;
    //         ino = ialloc(dev);    
    //         bno = balloc(dev);

    int ino = ialloc(pip->dev), bno = balloc(pip->dev);

    // 3. mip = iget(dev, ino) to load the inode into a minode[] (in order to
    // wirte contents to the INODE in memory).

    // 4. Write contents to mip->INODE to make it as a DIR INODE.

    // 5. iput(mip); which should write the new INODE out to disk.

    // // C CODE of (3), (4) and (5):
    // //**********************************************************************
    // mip = iget(dev,ino);
    // INODE *ip = &mip->INODE;
    // Use ip-> to acess the INODE fields:

    // i_mode = 0x41ED;		// OR 040755: DIR type and permissions
    // i_uid  = running->uid;	// Owner uid 
    // i_gid  = running->gid;	// Group Id
    // i_size = BLKSIZE;		// Size in bytes 
    // i_links_count = 2;	        // Links count=2 because of . and ..
    // i_atime = i_ctime = i_mtime = time(0L);  // set to current time
    // i_blocks = 2;                	// LINUX: Blocks count in 512-byte chunks 
    // i_block[0] = bno;             // new DIR has one data block   
    // i_block1] to i_block[14] = 0;
    
    // mip->dirty = 1;               // mark minode dirty
    // iput(mip);                    // write INODE to disk

    MINODE * mip = iget(dev, ino);
    ip = &mip->INODE; // this is KC's style of doing things
    ip->i_mode = 0x41ED;		// OR 040755: DIR type and permissions
    ip->i_uid  = running->uid;	// Owner uid 
    ip->i_gid  = running->gid;	// Group Id
    ip->i_size = BLKSIZE;		// Size in bytes 
    ip->i_links_count = 2;	        // Links count=2 because of . and ..
    ip->i_atime = ip->i_ctime = ip->i_mtime = time(0L);  // set to current time
    ip->i_blocks = 2;                	// LINUX: Blocks count in 512-byte chunks 
    ip->i_block[0] = bno;             // new DIR has one data block   
    int i = 0;
    for (i = 1; i < 15; i++) {
        ip->i_block[i] = 0;
    }

    mip->dirty = 1;
    iput(mip);

    // //***** create data block for new DIR containing . and .. entries ******
    // 6. Write . and .. entries into a buf[ ] of BLKSIZE

    // | entry .     | entry ..                                             |
    // ----------------------------------------------------------------------
    // |ino|12|1|.   |pino|1012|2|..                                        |
    // ----------------------------------------------------------------------

    char buf[BLKSIZE];

    // access space for .
    dp = (DIR *)buf;
    char * cp = buf;

    // initialize .
    dp->inode = ino;
    dp->rec_len = 12;
    dp->name[0] = '.';
    dp->name_len = 1;
    dp->file_type = 2; // DIR type has num 2

    // move to space for ..
    cp = cp + dp->rec_len;
    dp = (DIR *)cp;

    // initialize ..
    dp->inode = pip->ino;
    dp->rec_len = 1012;
    dp->name[0] = dp->name[1] = '.';
    dp->name_len = 2;
    dp->file_type = 2; // 2 for DIR type

    // Then, write buf[ ] to the disk block bno;

    put_block(dev, bno, buf);

    // 7. Finally, enter name ENTRY into parent's directory by 
    //             enter_name(pip, ino, name);

    return enter_dir_name(pip, ino, name);

    // 8. int enter_name(MINODE *pip, int myino, char *myname)
    // see above function for continued/further notes/details
}        

int make_dir(void) {
    // 1. pathname = "/a/b/c" start mip = root;         dev = root->dev;
    //             =  "a/b/c" start mip = running->cwd; dev = running->cwd->dev;

    MINODE * mip = NULL;
    if (pathname[0] == '/') {
        mip = root;
        dev = root->dev;
    }
    else {
        mip = running->cwd;
        dev = running->cwd->dev;
    }

    // 2. Let  
    //     parent = dirname(pathname);   parent= "/a/b" OR "a/b"
    //     child  = basename(pathname);  child = "c"

    // WARNING: strtok(), dirname(), basename() destroy pathname

    char path_parent_copy[256] = "", path_child_copy[256] = "";
    strcpy(path_parent_copy, pathname);
    strcpy(path_child_copy, pathname);
    char * parent = dirname(path_parent_copy);
    char * child = basename(path_child_copy);

    // 3. Get the In_MEMORY minode of parent:

    //         pino  = getino(parent);
    //         pip   = iget(dev, pino); 

    int pino = getino(&dev, parent);
    MINODE * pip = iget(dev, pino);

    // Verify : (1). parent INODE is a DIR (HOW?)   AND
    //             (2). child does NOT exists in the parent directory (HOW?);

    if (!S_ISDIR(pip->INODE.i_mode)) {
        printf("  make_dir(): error: %s is not a directory\n", pathname);
        iput(mip); // should do this because iget was called (?)
        iput(pip);
        return -3;
    }
    
    int found = search(&pip->INODE, child, dev);
    if (found) {
        printf("  make_dir(): error: %s already exists\n", pathname);
        iput(mip); // should do this because iget was called (?)
        iput(pip);
        return -5;
    }
                
    // 4. call mymkdir(pip, child);

    int result = my_mkdir(pip, child);

    // 5. inc parent inodes's link count by 1; 
    // touch its atime and mark it DIRTY

    pip->INODE.i_links_count++; // --> DIFFERENT from ref count
    pip->INODE.i_atime = pip->INODE.i_ctime = pip->INODE.i_mtime = time(0L); // sets it to the current time
    pip->dirty = 1;

    // 6. iput(pip);
     
    iput(pip);
    iput(mip); // should do this because iget was called (?)


    return result;
}

// Algorithm based on enter_dir_name(), differences noted in comments
int enter_reg_name(MINODE * pip, int new_ino, char * new_name) {
    int i = 0, ideal_len = 0, remaining = 0, new_name_len = strlen(new_name);
    int needed_len = 4 * ( (8 + new_name_len + 3) / 4 ), entry_entered = 0;
    char buf[BLKSIZE], * cp = NULL, temp = '\0';

    for (i = 0; i < 12; i++) {
        if (pip->INODE.i_block[i] == 0)
            break; // reached the end of currently allocated blocks

        get_block(pip->dev, pip->INODE.i_block[i], buf);
        dp = (DIR *)buf;
        cp = buf;

        printf("  enter_reg_name(): stepping to last entry in block %d\n", 
            pip->INODE.i_block[i]);

        while (cp + dp->rec_len < buf + BLKSIZE) { // if ==, at the end, don't advance
            temp = dp->name[dp->name_len]; // save character
            dp->name[dp->name_len] = '\0'; // temporarily make name a string
            printf("    enter_reg_name(): curr entry = %s\n",dp->name);
            dp->name[dp->name_len] = temp; // replace char with original

            cp += dp->rec_len;
            dp = (DIR *)cp;
        }
        // now dp points to the last entry in the current block

        ideal_len = 4 * ( (8 + dp->name_len + 3) / 4 );
        remaining = dp->rec_len - ideal_len;
        if (remaining >= needed_len) {
            // trim previous entry
            dp->rec_len = ideal_len;

            // advance to position for new entry
            cp += dp->rec_len;
            dp = (DIR *)cp;

            // write new entry to buf
            dp->inode = new_ino;
            dp->rec_len = buf + BLKSIZE - cp; // check this against remaining
            dp->name_len = new_name_len;
            strncpy(dp->name, new_name, new_name_len);
            dp->file_type = 1; // REG file has type 1

            entry_entered = 1;
            break;
        }
    }

    if (!entry_entered) {
        int bno = balloc(dev); // allocate a new disk block
        pip->INODE.i_size += BLKSIZE; // increment parent inode's size
        pip->INODE.i_block[i] = bno; // record block num in parent inode

        get_block(dev, bno, buf); // get the corresponding (new) block from disk
        dp = (DIR *)buf;
        dp->inode = new_ino;
        dp->rec_len = BLKSIZE;
        dp->name_len = new_name_len;
        strncpy(dp->name, new_name, new_name_len);
        dp->file_type = 1; // 1 for REG file

        entry_entered = 1;
    }

    if (!entry_entered) {
        printf("  enter_reg_name(): error adding name to parent inode\n");
        return -9;
    }

    // rmn extras
    pip->INODE.i_ctime = pip->INODE.i_mtime = time(0L);
    pip->dirty = 1;

    put_block(dev, pip->INODE.i_block[i], buf);

    return 1;
}

// Algorithm based on make_dir(), differences noted in comments
// On success, returns the allocated ino number
int my_creat(MINODE * pip, char * name) {
    // Only allocate ino, no disk block

    int ino = ialloc(dev);

    MINODE * mip = iget(dev, ino);
    ip = &mip->INODE;           // this is KC's style of doing things
    ip->i_mode = 0x81A4;		// OR 0100644: REG type and permissions
    ip->i_uid  = running->uid;	// Owner uid 
    ip->i_gid  = running->gid;	// Group Id
    ip->i_size = 0;	        	// Size in bytes --> 0 for empty file
    ip->i_links_count = 1;	    // Links count 1 for reg file (parent keeps track)
    ip->i_atime = ip->i_ctime = ip->i_mtime = time(0L);  // set to current time
    ip->i_blocks = 0;           // LINUX: Blocks count in 512-byte chunks --> ZERO for creat
    // No data block
    int i = 0;
    for (i = 0; i < 15; i++) { // start i at 0 as a result
        ip->i_block[i] = 0;
    }

    mip->dirty = 1;
    iput(mip);

    // No data block for file to process, finish with enter_reg_name()

    int result = enter_reg_name(pip, ino, name);
    if (result > 0) {
        return ino;
    }
    else {
        return result;
    }
}        

// Algorithm based on make_dir(), differences noted in comments
int create_file(void) {
    MINODE * mip = NULL;
    if (pathname[0] == '/') {
        mip = root;
        dev = root->dev;
    }
    else {
        mip = running->cwd;
        dev = running->cwd->dev;
    }

    char path_parent_copy[256] = "", path_child_copy[256] = "";
    strcpy(path_parent_copy, pathname);
    strcpy(path_child_copy, pathname);
    char * parent = dirname(path_parent_copy);
    char * child = basename(path_child_copy);

    int pino = getino(&dev, parent);
    MINODE * pip = iget(dev, pino);

    if (!S_ISDIR(pip->INODE.i_mode)) {
        printf("  create_file(): error: %s is not a directory\n", pathname);
        iput(mip);
        iput(pip);
        return -7;
    }
    
    int found = search(&pip->INODE, child, dev);
    if (found) {
        printf("  create_file(): error: %s already exists\n", pathname);
        iput(mip);
        iput(pip);
        return -8;
    }
                
    int result = my_creat(pip, child);

    // Do NOT increase parent's link count --> DIFFERENT from ref count
    pip->INODE.i_atime = pip->INODE.i_ctime = pip->INODE.i_mtime = time(0L); // sets it to the current time
    pip->dirty = 1;

    iput(mip);
    iput(pip);

    return result;
}

int rm_child_name(MINODE * pip, char * rm_entry_name) {
    // rm_child(): removes the entry [INO rlen nlen name] from parent's data block.

    // int rm_child(MINODE *parent, char *name) {
    //    1. Search parent INODE's data block(s) for the entry of name

    // Search alg based on util.c search()
    char dbuf[BLKSIZE];

    get_block(dev, pip->INODE.i_block[0], dbuf); // get the inode's /data/ block

    // Let DIR *dp and char *cp BOTH point at dbuf;
    // Use dp-> to access the fields of the record, e.g. print its name

    DIR * dp_prev = NULL;
    char * cp = NULL, * cp_prev = NULL, temp[256] = "";
    int i = 0, j = 0, found = 0;

    for (i = 0; i < 12; i++) { // for now assume only direct blocks
        if (pip->INODE.i_block[i] == 0) {
            // if this is 0, data not found or no more data
            break;
        }
        dp_prev = NULL;
        cp_prev = NULL;
        dp = (DIR *)dbuf;
        cp = dbuf;

        get_block(dev, pip->INODE.i_block[i], dbuf); // get the inode's /data/ block

        while (cp < dbuf + BLKSIZE) { // while still within the data block
            strncpy(temp, dp->name, dp->name_len);
            temp[dp->name_len] = '\0'; // NOTE: strncpy() does not guarantee '\0' append

            if (strcmp(temp, rm_entry_name) == 0) { // match found
                found = 1;
                break;
            }

            dp_prev = dp;
            cp_prev = cp;
            cp += dp->rec_len;
            dp = (DIR *)cp;
        }
        if (found == 1) { // needed to preserve i
            break;
        }
    }

    if (!found) {
        printf("  rm_child_name(): error: entry %s not found in parent"
            " dir (ino = %d)\n", rm_entry_name, pip->ino);
        return -14;
    }

    //    2. Erase name entry from parent directory by
        
    //   (1). if LAST entry in block{
    //                                          |remove this entry   |
    //           -----------------------------------------------------
    //           xxxxx|INO rlen nlen NAME |yyy  |zzz                 | 
    //           -----------------------------------------------------

    //                   becomes:
    //           -----------------------------------------------------
    //           xxxxx|INO rlen nlen NAME |yyy (add zzz len to yyy)  |
    //           -----------------------------------------------------

    //       }

    if (cp + dp->rec_len == dbuf + BLKSIZE) { // at last entry in block
        dp_prev->rec_len += dp->rec_len;
    }
        
    //   (2). if (first entry in a data block){
    //           deallocate the data block; modify parent's file size;

    //           -----------------------------------------------
    //           |INO Rlen Nlen NAME                           | 
    //           -----------------------------------------------
            
    //           Assume this is parent's i_block[i]:
    //           move parent's NONZERO blocks upward, i.e. 
    //                i_block[i+1] becomes i_block[i]
    //                etc.
    //           so that there is no HOLEs in parent's data block numbers
    //       }

    else if (dp->rec_len == BLKSIZE) { // only entry in block
        bdealloc(dev, pip->INODE.i_block[i]);
        pip->INODE.i_size -= BLKSIZE;
        while (pip->INODE.i_block[i + 1] != 0) {
            pip->INODE.i_block[i] = pip->INODE.i_block[i + 1];
        }
    }

    //   (3). if in the middle of a block{
    //           move all entries AFTER this entry LEFT;
    //           add removed rec_len to the LAST entry of the block;
    //           no need to change parent's fileSize;

    //                | remove this entry   |
    //           -----------------------------------------------
    //           xxxxx|INO rlen nlen NAME   |yyy  |zzz         | 
    //           -----------------------------------------------

    //                   becomes:
    //           -----------------------------------------------
    //           xxxxx|yyy |zzz (rec_len INC by rlen)          |
    //           -----------------------------------------------

    //       }

    else { // middle of a block
        int rm_rec_len = dp->rec_len;
        memcpy(cp, cp + rm_rec_len, dbuf + BLKSIZE - (cp + rm_rec_len) );
        //while (cp_prev < dbuf + BLKSIZE - rm_rec_len) { // move to last entry
        while (cp + dp->rec_len < dbuf + BLKSIZE - rm_rec_len) { // move to last entry
            //cp_prev = cp;
            //dp_prev = dp;
            cp += dp->rec_len;
            dp = (DIR *)cp;
        }
        // cp_prev now points to last entry, cp points randomly
        dp->rec_len += rm_rec_len;
    }
        
    //   3. Write the parent's data block back to disk;
    //      mark parent minode DIRTY for write-back
    // }

    // rmn extras
    pip->INODE.i_ctime = pip->INODE.i_mtime = time(0L);

    // does this need to happen if it was the only entry?
    put_block(pip->dev, pip->INODE.i_block[i], dbuf);
    pip->dirty = 1;
}

int rm_dir(void) {
    MINODE * mip = NULL;
    if (pathname[0] == '/') {
        mip = root;
        dev = root->dev;
    }
    else {
        mip = running->cwd;
        dev = running->cwd->dev;
    }
    // Are the above steps needed for anything other than the dev num?

    char path_parent_copy[256] = "", path_child_copy[256] = "";
    strcpy(path_parent_copy, pathname);
    strcpy(path_child_copy, pathname);
    char * parent = dirname(path_parent_copy);
    char * child = basename(path_child_copy);

    // Every file operation is performed by a process
    // At start, have two processes, P1 (with uid 0 for super user) and 
    //   P2 (uid 1, for ordinary user)
    // P1 runs first; P2 waits in a readyQueue, which contains processes 
    //   that are ready to run

    // To rmdir properly need to verify ownership; if current user is super 
    //   user, no need to check further; else, uid must match

    iput(mip);
    // Check dir type
    int ino = getino(&dev, pathname);
    mip = iget(dev, ino);

    if (!S_ISDIR(mip->INODE.i_mode)) {
        printf("  rmdir(): error: %s is not a directory\n", pathname);
        iput(mip);
        return -10;
    }
    
    // Check busy

    
    // Check empty
    if (mip->INODE.i_links_count > 2) {
        printf("  rmdir(): error: %s is not empty\n", pathname);
        iput(mip);
        return -12;
    }
    char buf[BLKSIZE], * cp = NULL;
    get_block(dev, mip->INODE.i_block[0], buf);
    dp = (DIR *)buf;
    cp = buf;

    // move from . to ..
    cp += dp->rec_len;
    dp = (DIR *)cp;
    if (dp->rec_len == 12) { // dir not empty
        printf("rmdir(): error: %s is not empty\n", pathname);
        iput(mip);
        return -13;
    }

    int i = 0;
    for (i = 0; i < 12; i++) {
        if (mip->INODE.i_block[i] == 0) {
            continue;
        }
        bdealloc(mip->dev, mip->INODE.i_block[i]);
    }
    idealloc(mip->dev, mip->ino);
    iput(mip); // makes mip->ref_count 0

    int pino = getino(&dev, parent);
    MINODE * pip = iget(dev, pino);

    rm_child_name(pip, child);

    pip->INODE.i_links_count--;
    pip->INODE.i_atime = pip->INODE.i_ctime =pip->INODE.i_mtime =time(0L);
    pip->dirty = 1;
    iput(pip);

    return 1;
}