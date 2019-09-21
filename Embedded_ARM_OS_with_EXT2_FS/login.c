/****************** Algorithm of login *******************/
// login.c : Upon entry, argv[0]=login, argv[1]=/dev/ttyX
#include "ucode.c"

int in, out, err;
char login_name[128], password[128];

main(int argc, char *argv[]) {
	// (1).close file descriptors 0, 1 inherited from INIT.
	close(0);
	close(1);

	// (2).open argv[1] 3 times as in(0), out(1), err(2).
	in = open(argv[1], O_RDONLY);
	out = open(argv[1], O_WRONLY);
	err = open(argv[1], O_WRONLY);
	
	// (3).settty(argv[1]); // set tty name string in PROC.tty
	fixtty(argv[1]);

	// (4). open /etc/passwd file for READ;
	int fd_pw, n, count = 0, i, entry, start;
	char pw_buf[BLKSIZE], line[256], * entry_ptrs[7] = { NULL }, cmdline[128] = "";

	while (1) {
		// (5).
		fd_pw = open("/etc/passwd", O_RDONLY);
		memset(login_name, 0, 128);
		memset(password, 0, 128);
		printf("login:");
		gets(login_name);
		printf("password:");
		gets(password);

		// To Implement: checking/saving & wrapping in case file is longer than buf
		n = read(fd_pw, pw_buf, BLKSIZE);
		// format is ==> username:password:gid:uid:fullname:HOMEDIR:program
		// for	each line in / etc / passwd file do
		while (count < n) {
			count += find_line(line, pw_buf + count, BLKSIZE - count);
			// tokenize user account line;
			for (i = 0, entry = 0, start = 0; entry < 7; i++) {
				if (line[i] == ':' || line[i] == '\0') {
					line[i] = '\0';
					entry_ptrs[entry++] = &line[start];
					start = i + 1;
				}
			}

			printf("  login: %s\n", entry_ptrs[0]);
			printf("  pw: %s\n", entry_ptrs[1]);

			// (6).if (user has a valid account)
			if (!strcmp(login_name, entry_ptrs[0]) && !strcmp(password, entry_ptrs[1])) { 
				// (7).change uid, gid to user's uid, gid;  // chuid() 
				chuid(atoi(entry_ptrs[3]), atoi(entry_ptrs[2]));

				//     change cwd to user's home DIR  // chdir() 
				chdir(entry_ptrs[5]);

				//     close opened / etc / passwd file  // close()
				close(fd_pw);

				// (8).exec to program in user account  // exec()
				//strcpy(cmdline, "/"); // this and + 1 below also works for w/e reason
				strcpy(cmdline + 0, entry_ptrs[6]);
				exec(cmdline);
			}
		}
		printf("login failed, try again\n");
		close(fd_pw);
	}
}