// Note: based off of more.c

#include "ucode.c"

int main(int argc, char * argv[]) {
	int fd_in, fd_out, n;
	char more_buf[BLKSIZE];

	if (argv[1] == NULL) {
		fd_in = 0;
		fd_out = 1;
	}
	else {
		fd_in = open(argv[1], O_RDONLY);
		fd_out = open(argv[2], O_WRONLY | O_CREAT);
	}

	if (fd_in < 0) {
		prints("  l2u: error, unable to open infile\n");
		return fd_in;
	}
	if (fd_out < 0) {
		prints("  l2u: error, unable to open outfile\n");
		return fd_out;
	}

	// // Try to get it to work
	// char tty_port[64];
	// gettty(tty_port);
	// int tty_out = open(tty_port, O_WRONLY);

	// while (n = read(file_desc, cat_buf, BLKSIZE)) {
	// 	if (n < 0) {
	// 		printns("nobytes\n", 8);
	// 	}
	// 	// printns("testing\n", 8);
	// 	printns(cat_buf, n);
	// 	// write(1, cat_buf, n);
	// 	// write(1, "\n\r", 2);
	// 	// write(tty_out, cat_buf, n);
	// 	// write(tty_out, "\n\r", 2);
	// }

	// ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ 
	// This is the only way I've been able to get it to work with input 
	//   redirect, but then it doesn't work from the keyboard
	char l2u_char;
	while (n = read(fd_in, &l2u_char, 1)) {
		if (l2u_char >= 'a' && l2u_char <= 'z') {
			l2u_char -= 'a' - 'A';
		}
		write(fd_out, &l2u_char, 1);
		if (l2u_char == '\n') {
			write(fd_out, "\r", 1);
		}
	}
	// ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ 

	int finished_in = close(fd_in);
	int finished_out = close(fd_out);
}