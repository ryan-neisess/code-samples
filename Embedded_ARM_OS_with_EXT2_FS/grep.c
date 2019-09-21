// Note: based off of l2u.c

#include "ucode.c"

// int has_pattern(char grep_line[], char pattern[]) {
// 	int len = strlen(pattern), i = 0, j = 0, contains = 1;

// 	for (i = 0; grep_line[i] != '\0'; i++) { // for each pos in the line
// 		contains = 1;
// 		for (j = 0; grep_line[i + j] != '\0'; j++) { // check for the pattern from here
// 			printf("  i == %d\n",i);
// 			if (grep_line[i + j] != pattern[j]) { // if no match
// 				contains = 0; // set false and break for efficieny
// 				break;
// 			}
// 		}
// 		if (contains == 1) {
// 			return 1;
// 		}
// 	}
// 	return 0;
// }

int has_pattern(char grep_line[], char pattern[]) {
	int len = strlen(pattern), i = 0, j = 0, contains = 1;

	for (i = 0; grep_line[i] != '\0'; i++) { // for each pos in the line
		if (strncmp(&grep_line[i], pattern, len) == 0) {
			return 1;
		}
	}
	return 0;
}

int main(int argc, char * argv[]) {
	int fd_in, fd_out, n;

	// input is either file or stdin (also works for redirected stdin)
	if (argv[2] == NULL) {
		fd_in = 0;
	}
	else {
		fd_in = open(argv[2], O_RDONLY);
	}

	// output is stdout (also works for redirected stdout)
	fd_out = 1;

	if (fd_in < 0) {
		prints("  grep: error, unable to open infile\n");
		return fd_in;
	}
	if (fd_out < 0) {
		prints("  grep: error, unable to open outfile\n");
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
	char grep_char;
	char grep_line[BLKSIZE];
	int i;
	while (n = read(fd_in, &grep_char, 1)) {
		//write(fd_out, &grep_char, 1);
		// if (grep_char == '\n') {
		// 	write(fd_out, "\r", 1);
		// }
		grep_line[i] = grep_char;
		//printf("  %d ", grep_char);
		if (grep_char == '\n') {
			grep_line[i + 1] = '\0';
			if (has_pattern(grep_line, argv[1])) {
				write(fd_out, grep_line, i);
				write(fd_out, "\n\r", 2);
			}
			//write(fd_out, grep_line, i);
			memset(grep_line, 0, i);
			i = 0;
		}
		else {
			i++;
		}
		//write(fd_out, grep_line, i);
	}
	// ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ 

	int finished_in = close(fd_in);
	int finished_out = close(fd_out);
}