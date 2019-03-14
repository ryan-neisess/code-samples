#include "util.c"

int rmn_stat(char filename[], struct stat * statbuf);
int change_mode(char filename[], char mode[]);
int ch_mod(void);
int update_time(char filename[]);

// returns 0 on success, -1 o.w.
int rmn_stat(char filename[], struct stat * statbuf) {
	int ino = getino(&dev, filename);
	if (ino < 0) {
		return -1;
	}
	MINODE * mip = iget(dev, ino);

	statbuf->st_dev = dev;
	statbuf->st_ino = ino;
	statbuf->st_atime = mip->INODE.i_atime;
	statbuf->st_blksize = BLKSIZE; // check this one
	statbuf->st_blocks = mip->INODE.i_blocks;
	statbuf->st_ctime = mip->INODE.i_ctime;
	statbuf->st_gid = mip->INODE.i_gid;
	statbuf->st_mode = mip->INODE.i_mode;
	statbuf->st_mtime = mip->INODE.i_mtime;
	statbuf->st_nlink = mip->INODE.i_links_count; // check this one
	statbuf->st_size = mip->INODE.i_size;
	statbuf->st_uid = mip->INODE.i_uid;

	iput(mip);
	return 0;
}

// Assumes octal permissions
int change_mode(char filename[], char mode[]) {
	int ino = getino(&dev, filename);
	if (ino < 0) {
		return -1;
	}
	MINODE * mip = iget(dev, ino);

	int add_mode = 0;
	sscanf(mode, "%o", &add_mode);
	mip->INODE.i_mode |= add_mode;

	mip->INODE.i_atime = mip->INODE.i_ctime = mip->INODE.i_mtime = time(0L);
	mip->dirty = 1;
	iput(mip);
	return 1;
}

int ch_mod(void) {
	if (args[1] == NULL) {
		printf("  ch_mod(): error: link source not specified\n");
		return -30;
	}
	if (args[2] == NULL) {
		printf("  ch_mod(): error: link name not specified\n");
		return -31;
	}
	return change_mode(args[1], args[2]);
}

// utime(filename) --> Modify atime of INODE
int update_time(char filename[]) {
	int ino = getino(&dev, filename);
	if (ino < 0) {
		return -1;
	}
	MINODE * mip = iget(dev, ino);
	
	mip->INODE.i_atime = time(0L);
	iput(mip);
	return 1;
}