// Note: this version of cat works better with redirection and pipes

#include "ucode.c"

// #include <sys/types.h>
// #include <unistd.h>
// use below instead to avoid redefinitions
#define SEEK_END 2

int main(int argc, char * argv[]) {
	int file_desc, n;
	char cat_buf[BLKSIZE];

	if (argv[1] == NULL) {
		file_desc = 0;

		// close(0);
		// file_desc = open("/dev/tty0", O_RDONLY);
		// printf("  fd == %d\n", file_desc);

		// while (1) {
		// 	gets(cat_buf);
		// 	prints(cat_buf);
		// 	prints("\n");

		// 	// int i;
		// 	// for (i = 0; cat_buf[i] != '\0'; i++) {
		// 	// 	printf("%d ", cat_buf[i]);
		// 	// }
		// 	// printf("\n");
		// }
	}
	else {
		file_desc = open(argv[1], O_RDONLY);
	}

	if (file_desc < 0) {
		prints("  cat: error, unable to open file\n");
		return file_desc;
	}

	// Try to get it to work
	char tty_port[64];
	gettty(tty_port);
	int tty_out = open(tty_port, O_WRONLY);

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
	int screen = open("/dev/tty0", O_WRONLY);
	char cat_char;
	while (n = read(file_desc, &cat_char, 1)) {
		// if (cat_char == '\r') {
		// 	write(1, "\n", 1);
		// } 
		write(1, &cat_char, 1);
		if (cat_char == '\n') {
			write(screen, "\r", 1);
		}
	}
	// ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ 

	int finished = close(file_desc);
}