Welcome to Ryan Neisess' Linux-compatible EXT2 file system!

To run and test the file system, please run the bash script "instr_rmnfs.sh"
	in a Linux environment.*

This program was designed under the guidance of Dr. Kung-Chi Wang, Professor 
	of Electrical Engineering and Computer Science at Washington State 
	University - Pullman.

This file system conforms to Linux's EXT2 file system specifications including, 
	but not limited to, exact file type support (e.g. REG and DIR), double 
	indirect blocks, support of basic file manipulation commands (e.g. mkdir, 
	rmdir, creat, ls, cd, etc.), support of file I/O commands (e.g. open, read, 
	cat, cp), and can both be mounted by and recognized by Linux.
	
Note that in favor of gaining a greater understanding of the EXT2 file system 
	as a whole, many of the commands support primarily their core functions, 
	and arguments/flags to the  commands may not be supported. Some commands, 
	e.g. ls, perform a variant by default, e.g. ls -l, for ease of testing and 
	proof of concept.

This program/file system is still in development as well, where current goals 
	include more robust support of soft and hard links, as well as 
	implementation of mv. Future goals include mounting file systems themselves 
	and permissions checking for files and operations.

* Note: Running this program with this command creates a virtual disk with only 
	a few directories and files for ease of testing. However, to test/make full 
	use of the single and double indirect blocks support, either of the scripts 
	"lv2test.sh" or "" may be run instead.
	