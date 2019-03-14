#include "util.c"

extern char * args[12];
extern int enter_reg_name(MINODE * pip, int new_ino, char * new_name);
extern int bdealloc(int dev, int bno);
extern int idealloc(int dev, int ino);
extern int rm_child_name(MINODE * pip, char * old_name);
extern int my_creat(MINODE * pip, char * name);

int link_file(char old_file[], char new_file[]);
int rmn_link(void);

void rmn_truncate(MINODE * ulino);
int un_link(void);

int sym_link(char old_file[], char new_file[]);
int rmn_symlink(void);

char * read_link(char link_name[]);

int link_file(char old_file[], char new_file[]) {
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

    char parent_copy[256] = "", child_copy[256] = "";
    strcpy(parent_copy, new_file);
    strcpy(child_copy, new_file);
    char * parent = dirname(parent_copy);
    char * child = basename(child_copy);

    // Check dir type
    int old_ino = getino(&dev, old_file);
    MINODE * old_mip = iget(dev, old_ino);

    if (S_ISDIR(old_mip->INODE.i_mode)) {
        printf("  link(): error: %s is a directory\n", old_file);
		iput(mip);
		iput(old_mip);
        return -15;
    }

	int new_pino = getino(&dev, parent);
	MINODE * new_pip = iget(dev, new_pino);

	if (!S_ISDIR(new_pip->INODE.i_mode)) {
        printf("  link(): error: %s is not a directory\n", parent);
		iput(mip);
		iput(old_mip);
		iput(new_pip);
        return -16;
    }

	if (old_mip->dev != new_pip->dev) {
		printf("  link(): error: %s and %s are not on the same device\n", 
			old_file, new_file);
		return -17;
	}

	int found = search(&new_pip->INODE, child, dev);
    if (found) {
        printf("  link(): error: %s already exists\n", pathname);
		iput(mip);
		iput(old_mip);
		iput(new_pip);
        return -18;
    }

	enter_reg_name(new_pip, old_ino, child); // reuse function from creat
	old_mip->INODE.i_links_count++;
	old_mip->dirty = 1;
	iput(old_mip);
	iput(new_pip);
	return 1;
}

int rmn_link(void) {
	if (args[1] == NULL) {
		printf("  rmn_link(): error: link source not specified\n");
		return -19;
	}
	if (args[2] == NULL) {
		printf("  rmn_link(): error: link name not specified\n");
		return -20;
	}
	return link_file(args[1], args[2]);
}

void rmn_truncate(MINODE * ulino) {
	// deallocate direct blocks
	int i = 0;
    for (i = 0; i < 12 && ulino->INODE.i_block[i] != 0; i++) {
        bdealloc(dev, ulino->INODE.i_block[i]);
    }

    if (ip->i_block[12] != 0) { // look for single indirect blocks
        char ind_buf[BLKSIZE] = "";

        get_block(dev, ip->i_block[12], ind_buf);
        int * blockptr = (int *) ind_buf;

        for (i = 0; i < 256 && blockptr[i] != 0; i++) {
            bdealloc(dev, blockptr[i]);
        }
    }

    if (ip->i_block[13] != 0) { // look for double indirect blocks
        char ind_buf[BLKSIZE] = "", dbl_buf[BLKSIZE] = "";
        get_block(dev, ulino->INODE.i_block[13], ind_buf);
        int * blockptr = (int *) ind_buf, * dbl_blk_ptr = NULL, j = 0;

        for (i = 0; i < 1 && blockptr[i] != 0; i++) {
            get_block(dev, blockptr[i], dbl_buf);
            dbl_blk_ptr = (int *) dbl_buf;

            for (j = 0; j < 256 && dbl_blk_ptr[j] != 0; j++) {
                bdealloc(dev, dbl_blk_ptr[j]);
            }
        }
    }

	ulino->INODE.i_atime = ulino->INODE.i_ctime = ulino->INODE.i_mtime = time(0L);
	ulino->INODE.i_size = 0;
	ulino->dirty = 1;
}

int un_link(void) {
    MINODE * mip = NULL;
    if (pathname[0] == '/') {
        dev = root->dev;
    }
    else {
        dev = running->cwd->dev;
    }

    char path_parent_copy[256] = "", path_child_copy[256] = "";
    strcpy(path_parent_copy, pathname);
    strcpy(path_child_copy, pathname);
    char * parent = dirname(path_parent_copy);
    char * child = basename(path_child_copy);

    int ino = getino(&dev, pathname), pino = getino(&dev, parent);
    MINODE * pip = iget(dev, pino);
	mip = iget(dev, ino);

	if (S_ISDIR(mip->INODE.i_mode)) {
        printf("  un_link(): error: %s is not a directory\n", pathname);
		iput(mip);
        iput(pip);
        return -21;
    }

	mip->INODE.i_links_count--;
	if (mip->INODE.i_links_count == 0) {
		rmn_truncate(mip);
		idealloc(dev, mip->ino);
	}

	rm_child_name(pip, child);
	iput(mip);
	iput(pip);
	return 1;
}

// Assume old_file is 60 chars max, incl '\0'
int sym_link(char old_name[], char new_name[]) {
    if (pathname[0] == '/') {
        dev = root->dev;
    }
    else {
        dev = running->cwd->dev;
    }

    char parent_copy[256] = "", child_copy[256] = "";
    strcpy(parent_copy, new_name);
    strcpy(child_copy, new_name);
    char * parent = dirname(parent_copy);
    char * child = basename(child_copy);

    // Check dir type
    int old_ino = getino(&dev, old_name);
    MINODE * old_mip = iget(dev, old_ino);

	if (old_ino < 0) {
		printf("  sym_link(): error: %s not found\n", old_name);
		iput(old_mip);
        return -24;
	}

    if (!S_ISDIR(old_mip->INODE.i_mode) && 
		!S_ISREG(old_mip->INODE.i_mode) && 
		!S_ISLNK(old_mip->INODE.i_mode)) {

        printf("  sym_link(): error: %s is not dir, reg, or lnk\n", old_name);
		iput(old_mip);
        return -25;
    }

	int new_pino = getino(&dev, parent);
	MINODE * new_pip = iget(dev, new_pino);

	if (!S_ISDIR(new_pip->INODE.i_mode)) {
        printf("  sym_link(): error: %s is not a directory\n", parent);
		iput(old_mip);
		iput(new_pip);
        return -26;
    }

	int found = search(&new_pip->INODE, child, dev);
    if (found) {
        printf("  sym_link(): error: %s already exists\n", new_name);
		iput(old_mip);
		iput(new_pip);
        return -27;
    }

	int new_ino = my_creat(new_pip, child);
	// new_ino will have newly allocated ino on success

	if (new_ino < 0) {
		printf("  sym_link(): error: failed to create %s\n", pathname);
		iput(old_mip);
		iput(new_pip);
        return -28;
	}

	MINODE * new_mip = iget(dev, new_ino);

	// change file type to lnk
	// new_mip->INODE.i_mode &= 0x0FFF;
	// new_mip->INODE.i_mode |= 0xA000;
	new_mip->INODE.i_mode &= 0xFFFF;

	strcpy((char *)new_mip->INODE.i_block, old_name);
	new_mip->INODE.i_size = strlen(old_name) + 1; // + 1 for '\0'

	iput(old_mip);
	iput(new_mip);
	iput(new_pip);

	return 1;
}

int rmn_symlink(void) {
	if (args[1] == NULL) {
		printf("  rmn_symlink(): error: link source not specified\n");
		return -22;
	}
	if (args[2] == NULL) {
		printf("  rmn_symlink(): error: link name not specified\n");
		return -23;
	}
	return sym_link(args[1], args[2]);
}

char * read_link(char link_name[]) {
	int ino = getino(&dev, link_name);
	MINODE * mip = iget(dev, ino);

	if (!S_ISLNK(mip->INODE.i_mode)) {
		printf("  read_link(): error: %s is not symlink\n", link_name);
		iput(mip);
        return NULL;
	}

	return (char *)mip->INODE.i_block;
}