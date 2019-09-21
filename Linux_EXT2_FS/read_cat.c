#include "util.c"

extern int rmn_open(void);
extern int rmn_close(void);

int rmn_read(int file_desc, char *buf, int nbytes) {
	if (nbytes > BLKSIZE) {
		printf("  rmn_read(): warning: nbytes > standard BLKSIZE, undefined behavior may result\n");
	}

	int count = 0, blk = 0;

	// number of bytes still available in file.
    int avail = running->fd[file_desc]->mptr->INODE.i_size - running->fd[file_desc]->offset;
    
	int lbk, startByte, remain, bytes_to_read;
	char readbuf[BLKSIZE];

	while (nbytes > 0 && avail > 0){
		lbk = running->fd[file_desc]->offset / BLKSIZE;
		startByte = running->fd[file_desc]->offset % BLKSIZE;
     
		// Note: lbk num starts at 0

		if (lbk < 12){ // lbk is a direct block
	   		// map LOGICAL lbk to PHYSICAL blk
			// lbk numbers 0 through 11 (count == 12) are within direct blocks
        	blk = running->fd[file_desc]->mptr->INODE.i_block[lbk];
		}
		else if (lbk >= 12 && lbk < 256 + 12) { 
			// indirect blocks 
			// Note: there are 256 disk block numbers in the single indirect block
			// Thus, lbk numbers 12 (aka 0 + 12) through 267 (aka 256 - 1 + 12) are 
			//   found in the single indirect blocks
			char ind_buf[BLKSIZE];

			get_block(running->fd[file_desc]->mptr->dev, 
				running->fd[file_desc]->mptr->INODE.i_block[12], ind_buf);

			int * ind_buf_ptr = (int *)ind_buf;
			blk = ind_buf_ptr[lbk - 12];
		}
		else { 
			// double indirect blocks
			char dbl_ind_buf[BLKSIZE];

			get_block(running->fd[file_desc]->mptr->dev, 
				running->fd[file_desc]->mptr->INODE.i_block[13], dbl_ind_buf);

			char second_buf[BLKSIZE];
			int dbl_ind_blk_num = (lbk - 268) / 256;
			int * dbl_ind_buf_ptr = (int *)dbl_ind_buf;

			get_block(running->fd[file_desc]->mptr->dev, 
				dbl_ind_buf_ptr[dbl_ind_blk_num], second_buf);

			int second_ind_blk_num = (lbk - 268) % 256;
			int * second_buf_ptr = (int *)second_buf;
			blk = second_buf_ptr[second_ind_blk_num];
		}

		//printf("  lbk: %d\n  disk blk: %d\n", lbk, blk);

		/* get the data block into readbuf[BLKSIZE] */
		get_block(running->fd[file_desc]->mptr->dev, blk, readbuf);

		/* copy from startByte to buf[ ], at most remain bytes in this block */
		remain = BLKSIZE - startByte;   // number of bytes remain in readbuf[]
		
		if (avail < remain) {
			bytes_to_read = avail;
		}
		else {
			bytes_to_read = remain;
		}

		memcpy(buf, readbuf + startByte, bytes_to_read);

		running->fd[file_desc]->offset += bytes_to_read;
		avail -= bytes_to_read;
		count += bytes_to_read;
		remain -= bytes_to_read;

		if (remain <= 0) {
			break;
		}
	
		// if one data block is not enough, loop back to OUTER while for more ...
	}
	//printf("  rmn_read(): read %d char from file descriptor %d\n", count, fd);  
	return count;   // count is the actual number of bytes read
}

int read_file(void) {
	if (args[1] == NULL) {
		printf("  read_file(): error: fd not specified\n");
		return -45;
	}
	if (args[2] == NULL) {
		printf("  read_file(): error: n bytes not specified\n");
		return -46;
	}

	int file_fd = atoi(args[1]), nbytes = atoi(args[2]);

	if (running->fd[file_fd] == NULL) {
		printf("  read_file(): error: %d not allocated\n", file_fd);
		return -47;
	}
	if (running->fd[file_fd]->mode != 0 || running->fd[file_fd]->mode != 1) {
		printf("  read_file(): error: %d not open for read compatible mode\n", file_fd);
		return -48;
	}

	char buf[BLKSIZE];
	return rmn_read(file_fd, buf, nbytes);
}

int rmn_cat(void) {
	if (args[1] == NULL) {
		printf("  read_cat(): error: filename not specified\n");
		return -49;
	}

	args[2] = (char *)malloc(sizeof(char) * 2);
	args[2][0] = '0';
	args[2][1] = '\0';

	int file_desc = rmn_open(), n;
	if (file_desc < 0) {
		return file_desc;
	}
	char cat_buf[BLKSIZE];

	int x = 0;
	while (n = rmn_read(file_desc, cat_buf, BLKSIZE)) {
		printf("%.*s", n, cat_buf); // will print only up to n bytes
		printf("\n%d - - - - - - - - - - - -\n", ++x);
	}

	args[1][0] = file_desc + '0';
	args[1][1] = '\0';

	int finished = rmn_close();
}