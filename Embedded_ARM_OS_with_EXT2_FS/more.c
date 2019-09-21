// Note: based off of cat.c

#include "ucode.c"

int main(int argc, char * argv[]) {
	int file_desc, n;
	char more_buf[BLKSIZE];

	if (argv[1] == NULL) { // for more, assumes input redirection
		file_desc = 0;
	}
	else {
		file_desc = open(argv[1], O_RDONLY);
	}

	if (file_desc < 0) {
		prints("  more: error, unable to open file\n");
		return file_desc;
	}

	// ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ 
	// This is the only way I've been able to get it to work with input 
	//   redirect, but then it doesn't work from the keyboard
	int kbd_fd = open("/dev/tty0", O_RDONLY);
	char more_char, catch = ' ';
	int nl_count = 0, threshold = 25;
	while (n = read(file_desc, &more_char, 1)) {
		write(1, &more_char, 1);
		if (more_char == '\n') {
			write(1, "\r", 1);
			nl_count++;
		}
		if (nl_count >= threshold) {
			read(kbd_fd, &catch, 1);
			if (catch == ' ') {
				threshold = 25;
			}
			else if (catch == 'q') {
				break;
			}
			else { // if (catch == '\n') {
				threshold = 1;
			}
			nl_count = 0;
		}
	}
	// ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ 

	int finished = close(file_desc);
}