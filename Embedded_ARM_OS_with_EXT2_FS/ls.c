#include "ucode.c"

char *t1 = "xwrxwrxwr-------";
char *t2 = "----------------";
struct stat mystat, *sp;

int ls_file(char *fname) { // list a single file
	struct stat fstat, *sp = &fstat;
	int r, i;
	r = stat(fname, sp); // lstat the file

	if ((sp->st_mode & 0xF000) == 0x4000)
		printf("%c", 'd'); // print file type as d
	if ((sp->st_mode & 0xF000) == 0x8000)
		printf("%c", '-'); // print file type as -
	if ((sp->st_mode & 0xF000) == 0xA000)
		printf("%c", 'l'); // print file type as l

	for (i = 8; i >= 0; i--)
	{
		if (sp->st_mode & (1 << i))
			printf("%c", t1[i]); // print permission bit as r w x
		else
			printf("%c", t2[i]); // print permission bit as -
	}
	printf(" %d ", sp->st_nlink); // link count
	printf("%d ", sp->st_uid); // uid
	printf("%d ", sp->st_size); // file size
	printf(" %s\n", fname); // file basename
}

int ls_dir(char *dname) { // list a DIR
	char name[256]; // EXT2 filename: 1-255 chars
	char temp, tempstr[256];
	DIR *dp;
	int name_len;

	// open DIR to read names
	char dir_entries[BLKSIZE], cwd[256], * cp = dir_entries;
	getcwd(cwd);
	int fd = open(cwd, O_RDONLY), n = read(fd, dir_entries, BLKSIZE);

	while (cp < dir_entries + BLKSIZE) { 
		dp = (DIR *)cp;
		cp += dp->rec_len;

		strncpy(name, dp->name, dp->name_len);
		name[dp->name_len] = '\0';
		if (!strcmp(name, ".") || !strcmp(name, ".."))
			continue; // skip over . and .. entries
		strcpy(name, dname);
		strcat(name, "/");
		name_len = strlen(name);
		strncat(name, dp->name, dp->name_len);
		name[name_len + dp->name_len] = '\0';

		ls_file(name); // call list_file()
		//cp += dp->rec_len;
	}
}

int main(int argc, char *argv[]) {
	struct stat mystat, *sp;
	int r;
	char *s, sbuf[1024];
	char filename[1024], cwd[1024];

	//s = argv[1];   // ls [filename]
	if (argc == 1) // no parameter: ls CWD
		strcpy(sbuf, "./");
	else
		strcpy(sbuf, argv[1]);

	sp = &mystat;
	if ((r = stat(sbuf, sp)) < 0) { // stat() syscall
		exit(1);
	}

	// strcpy(filename, s);
	// if (s[0] != '/') { // filename is relative to CWD
	// 	getcwd(cwd); // get CWD path
	// 	strcpy(filename, cwd);
	// 	strcat(filename, "/");
	// 	strcat(filename, s); // construct $CWD/filename
	// }

	if ((sp->st_mode & 0xF000) == 0x4000)
		ls_dir(filename); // list DIR
	else
		ls_file(filename); // list single file
}