#include "util.c"

// Assume mode is 0, 1, 2, or 3 for R, W, RW, or APPEND, respectively
int rmn_open(void){
	if (args[1] == NULL) {
		printf("  rmn_open(): error: file not specified\n");
		return -32;
	}
	if (args[2] == NULL) {
		printf("  rmn_open(): error: mode not specified\n");
		return -33;
	}

	printf("    args[1]: %s\n", args[1]);
	printf("    args[2]: %s\n", args[2]);
	
	if (pathname[0] == '/') {
        dev = root->dev;
    }
    else {
        dev = running->cwd->dev;
    }
	int ino = getino(&dev, args[1]);
	MINODE * mip = iget(dev, ino);

	if(!S_ISREG(mip->INODE.i_mode)) {
		printf("  rmn_open(): error: %s not reg file\n", args[1]);
		iput(mip);
		return -34;
	}

	int i = 0, file_mode = atoi(args[2]);
	for (i = 0; i < NFD; i++) {
		if (running->fd[i] != NULL && running->fd[i]->refCount > 0) {
			// if (file_mode == running->fd[i]->mode && file_mode == 0) {
			// 	// if modes are same AND mode(s) is/are read, allow
			// 	continue;
			// }
			// else {
			// 	printf("  rmn_open(): error: %s already open; all modes not R (read)\n", args[1]);
			// 	iput(mip);
			// 	return -35;
			// }
		}
	}

	OFT * oftp = (OFT *)malloc(sizeof(OFT));
	oftp->mode = file_mode;      // mode = 0|1|2|3 for R|W|RW|APPEND 
	oftp->refCount = 1;
	oftp->mptr = mip;  // point at the file's minode[]

	switch(file_mode){
		case 0: oftp->offset = 0;     // R: offset = 0
			break;
		case 1: rmn_truncate(mip);        // W: truncate file to 0 size
			oftp->offset = 0;
			break;
		case 2: oftp->offset = 0;     // RW: do NOT truncate file
			break;
		case 3: oftp->offset = mip->INODE.i_size;  // APPEND mode
			break;
		default: printf("  rmn_open(): error: %d is invalid mode\n", file_mode);
			iput(mip);
			return -36;
	}

	for (i = 0; i < NFD; i++) {
		if (running->fd[i] == NULL) {
			running->fd[i] = oftp;
			break; // preserve value of i
		}
	}

	if (i == 10) {
		printf("  rmn_open(): error: no available fd's\n");
		iput(mip);
		return -37;
	}

	mip->INODE.i_atime = time(0L);
	if (file_mode != 0) {
		mip->INODE.i_mtime = time(0L);
	}
	mip->dirty = 1;
	
	// DO NOT call iput() here; call in close instead

	return i; // use this as fd num
}

int rmn_close(void) {
	if (args[1] == NULL) {
		printf("  rmn_close(): error: fd not specified\n");
		return -38;
	}

	int file_desc = atoi(args[1]);

	if (file_desc < 0 || file_desc > 9) {
		printf("  rmn_close(): error: fd out of range\n");
		return -39;
	}

	if (running->fd[file_desc] == NULL) {
		printf("  rmn_close(): error: fd unallocated\n");
		return -40;
	}

	OFT * oftp = running->fd[file_desc];
	running->fd[file_desc] = 0;
	oftp->refCount--;
	if (oftp->refCount > 0) {
		return 0;
	}
	
	// When done, put MINODE back
	iput (oftp->mptr);

	return 0;
}

int rmn_lseek(void) {
	if (args[1] == NULL) {
		printf("  rmn_lseek(): error: fd not specified\n");
		return -41;
	}
	if (args[2] == NULL) {
		printf("  rmn_lseek(): error: position not specified\n");
		return -42;
	}

	int file_fd = atoi(args[1]), file_pos= atoi(args[2]);

	if (running->fd[file_fd] == NULL) {
		printf("  rmn_lseek(): error: fd %d not open\n", file_fd);
		return -43;
	}

	if (file_pos < 0 || file_pos > running->fd[file_fd]->mptr->INODE.i_size) {
		printf("  rmn_lseek(): error: position out of bounds\n");
		return -44;
	}

	int original_pos = running->fd[file_fd]->offset;
	running->fd[file_fd]->offset = file_pos;

	return original_pos;
}

int pfd(void) {
	printf(" fd      mode      offset       INODE\n");
	printf("----    ------    --------    ---------\n");
	
	int i = 0;
	for (i = 0; i < NFD; i++) {
		printf(" %2d     ", i);
		if (running->fd[i] != NULL) {
			switch(running->fd[i]->mode) {
				case 0: printf("%6s", "READ");
					break;
				case 1:printf("%6s", "WRITE");
					break;
				case 2:printf("%6s", "RW");
					break;
				case 3:printf("%6s", "APPEND");
					break;
			}
			printf("    %8d    [%2d, %3d]\n",running->fd[i]->offset, 
				running->fd[i]->mptr->dev, running->fd[i]->mptr->ino);
		}
		else {
			putchar('\n');
		}
	}
	putchar('\n');
	return 1;
}