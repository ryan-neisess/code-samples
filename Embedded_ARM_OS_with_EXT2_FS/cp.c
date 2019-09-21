#include "ucode.c"

int main(int argc, char * argv[]) {
	if (argv[1] == NULL || argv[2] == NULL) {
		printf("  cp: error, filename not specified\n");
	}

	char cp_buf[BLKSIZE];
	int n;
	int fd_src = open(argv[1], O_RDONLY);
	int fd_dest = open(argv[2], O_RDWR); // will test if file exists

	if (fd_dest < 0) { // dest file does not exist
		creat(argv[2]);
		fd_dest = open(argv[2], O_RDWR); // try to open file again
	}

	while (n = read(fd_src, cp_buf, BLKSIZE)) {
		write(fd_dest, cp_buf, n); // Note n and not BLKSIZE
	}

	int src_closed = close(fd_src);
	int dest_closed = close(fd_dest);
}