#include "util.c"

extern int rmn_open(void);
extern int rmn_close(void);
extern int balloc(int dev);
extern int create_file();

int rmn_write(int file_desc, char buf[], int nbytes) {
	OFT * oftp = running->fd[file_desc];
	MINODE * fmip = oftp->mptr;

	if (nbytes > BLKSIZE) {
		printf("  rmn_read(): warning: nbytes > standard BLKSIZE, undefined behavior may result\n");
	}

	int count = 0, blk = 0;
	int lbk, startByte, remain, buf_offset = 0, bytes_to_write;
	char writebuf[BLKSIZE];

	while (nbytes > 0) {
		//compute LOGICAL BLOCK(lbk) and the startByte in that lbk :
		lbk = oftp->offset / BLKSIZE;
		startByte = oftp->offset % BLKSIZE;
     
		// Note: lbk num starts at 0

		if (lbk < 12){ // lbk is a direct block
	   		// map LOGICAL lbk to PHYSICAL blk
			// lbk numbers 0 through 11 (count == 12) are within direct blocks

			if (fmip->INODE.i_block[lbk] == 0) { // if no data block yet
				fmip->INODE.i_block[lbk] = balloc(fmip->dev); // MUST ALLOCATE a block
				//fmip->INODE.i_blocks += 2;
			}
			blk = fmip->INODE.i_block[lbk];
		}
		else if (lbk >= 12 && lbk < 256 + 12) { // indirect blocks 
			// Note: there are 256 disk block numbers in the single indirect block
			// Thus, lbk numbers 12 (aka 0 + 12) through 267 (aka 256 - 1 + 12) are 
			//   found in the single indirect blocks
			char buf12[BLKSIZE];

			if (fmip->INODE.i_block[12] == 0) {
				fmip->INODE.i_block[12] = balloc(fmip->dev);

				get_block(fmip->dev, fmip->INODE.i_block[12], buf12);
				memset(buf12, 0, BLKSIZE);
				put_block(fmip->dev, fmip->INODE.i_block[12], buf12);
			}

			get_block(fmip->dev, fmip->INODE.i_block[12], buf12);

			int * buf12_ptr = (int *)buf12;

			if (buf12_ptr[lbk - 12] == 0) {
				buf12_ptr[lbk - 12] = balloc(fmip->dev);
				//fmip->INODE.i_blocks += 2;
			}
			blk = buf12_ptr[lbk - 12];

			put_block(fmip->dev, fmip->INODE.i_block[12], buf12);

			printf("\n\niblock[12], blk = %d, %d\n", fmip->INODE.i_block[12], blk);
			// getchar();
		}
		else { // double indirect blocks
			char buf13[BLKSIZE];

			if (fmip->INODE.i_block[13] == 0) {
				fmip->INODE.i_block[13] = balloc(fmip->dev);

				get_block(fmip->dev, fmip->INODE.i_block[13], buf13);
				memset(buf13, 0, BLKSIZE);
				put_block(fmip->dev, fmip->INODE.i_block[13], buf13);
			}

			get_block(fmip->dev, fmip->INODE.i_block[13], buf13);


			char dblk_buf[BLKSIZE];
			int dblk_num = (lbk - 268) / 256;
			int * dblk_ptr = (int *)buf13;

			if (dblk_ptr[dblk_num] == 0) {
				dblk_ptr[dblk_num] = balloc(fmip->dev);

				get_block(fmip->dev, dblk_ptr[dblk_num], dblk_buf);
				memset(dblk_buf, 0, BLKSIZE);
				put_block(fmip->dev, dblk_ptr[dblk_num], dblk_buf);
				put_block(fmip->dev, fmip->INODE.i_block[13], buf13); // blk num was changed!
			}

			get_block(fmip->dev, dblk_ptr[dblk_num], dblk_buf);

			int data_dblk_num = (lbk - 268) % 256;
			int * data_dblk_ptr = (int *)dblk_buf;

			if (data_dblk_ptr[data_dblk_num] == 0) {
				data_dblk_ptr[data_dblk_num] = balloc(fmip->dev);
				//fmip->INODE.i_blocks += 2;
			}
			blk = data_dblk_ptr[data_dblk_num];

			put_block(fmip->dev, dblk_ptr[dblk_num], dblk_buf);
		}

		/* get the data block into writebuf[BLKSIZE] */
		get_block(fmip->dev, blk, writebuf);

		/* all cases come to here : write to the data block */
		remain = BLKSIZE - startByte;   // number of BYTEs remain in this block
		buf_offset %= 1024;

		if (nbytes < remain) {
			bytes_to_write = nbytes;
		}
		else {
			bytes_to_write = remain;
		}

		// printf("    lbk: %d\n", lbk);
		// printf("    blk: %d\n", blk);
		// printf("    startByte: %d\n", startByte);
		// printf("    buf_offset: %d\n", buf_offset);
		// printf("    bytes_to_write: %d\n\n", bytes_to_write);

		memcpy(writebuf + startByte, buf + buf_offset, bytes_to_write);

		// printf(" * * * * * * * * * * * * writebuf * * * * * * * * * * * *\n");
		// printf("%s", writebuf);

		oftp->offset += bytes_to_write;
		nbytes -= bytes_to_write;
		count += bytes_to_write;
		buf_offset += bytes_to_write;

		if (oftp->offset > fmip->INODE.i_size) // especially for RW|APPEND mode
			fmip->INODE.i_size = oftp->offset;   // inc file size (if offset > fileSize)

		put_block(fmip->dev, blk, writebuf); // write wbuf[ ] to disk
	}

	fmip->dirty = 1; // mark mip dirty for iput()
	//printf("wrote %d char into file descriptor fd=%d\n", count, file_desc);
	return count;
}

int write_file(void) {
	if (args[1] == NULL) {
		printf("  write_file(): error: fd not specified\n");
		return -50;
	}
	if (args[2] == NULL) {
		printf("  write_file(): error: n bytes not specified\n");
		return -51;
	}

	int file_fd = atoi(args[1]), nbytes = atoi(args[2]);

	if (running->fd[file_fd] == NULL) {
		printf("  write_file(): error: %d not allocated\n", file_fd);
		return -52;
	}
	if (running->fd[file_fd]->mode != 1 || 
		running->fd[file_fd]->mode != 2 || 
		running->fd[file_fd]->mode != 3) {

		printf("  write_file(): error: %d not open for write compatible mode\n", file_fd);
		return -53;
	}

	char buf[BLKSIZE];
	return rmn_write(file_fd, buf, nbytes);
}

int rmn_cp(void) {
	if (args[1] == NULL || args[2] == NULL) {
		printf("  rmn_cp(): error: filename(s) not specified\n");
		return -54;
	}
	
	char src[64] = "", dest[64] = "";
	strcpy(src, args[1]);
	strcpy(dest, args[2]);

	printf("    src: %s\n", src);
	printf("    dest: %s\n", dest);

	// Create the dest file if not already existing
	strcpy(pathname, dest);
	int creat_result = create_file(); // will return neg number if file already exists or other error

	printf("    creat_result: %d\n", creat_result);

	char user_args[256] = "";

	// Open the source file for R
	strcpy(user_args, src);
	strcpy(user_args + strlen(user_args) + 1, "0");
	args[1] = user_args;
	args[2] = user_args + strlen(user_args) + 1;
	int src_fd = rmn_open();
	if (src_fd < 0) {
		return src_fd;
	}

	// Open the dest file for RW
	strcpy(user_args, dest);
	strcpy(user_args + strlen(user_args) + 1, "2");
	args[1] = user_args;
	args[2] = user_args + strlen(user_args) + 1;
	int dest_fd = rmn_open(), n;
	if (dest_fd < 0) {
		return dest_fd;
	}

	printf("  src: %d, dest: %d\n", src_fd, dest_fd);

	char cp_buf[BLKSIZE];
	int x = 0;
	while (n = rmn_read(src_fd, cp_buf, BLKSIZE)) {
		rmn_write(dest_fd, cp_buf, n); // Note n and not BLKSIZE
		x++;
	}
	printf("\n\n%d\n\n", x);

	args[1][0] = src_fd + '0';
	args[1][1] = '\0';
	int src_closed = rmn_close();

	args[1][0] = dest_fd + '0';
	args[1][1] = '\0';
	int dest_closed = rmn_close();
}
