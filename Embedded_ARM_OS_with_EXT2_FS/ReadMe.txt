Welcome to Ryan Neisess' Embedded ARM OS!

This project runs an ARM-based operating system running in a simulated/virtual 
	environment through QEMU.

This program was designed under the guidance of Dr. Kung-Chi Wang, Professor 
	of Electrical Engineering and Computer Science at Washington State 
	University - Pullman.
	
It includes an EXT2 file system with support for all major file system and 
	file I/O operations except advanced mounting (the system can be mounted by 
	Linux, however) coded entirely from the ground up and using the exact EXT2 
	file system C libraries libraries.

Booting and login have both been coded based on given starter code, while 
	process management has been coded from scratch and can be performed using 
	the built-in shell, which was also built from scratch.

While typical commands (think: programs) on Linux cannot be run, core versions 
	of widely used commands have been implmented from scratch (though may not 
	support all options as in Linux) including grep, cat, cp, etc.

For all supported commands, input redirection, output redirection for writing, 
	output redirection for appending, and multiple pipes are supported, with 
	every command utilizing fork() to run in its own process.

To run the project on Linux: 

1) Open up an additional terminal and run the "ps" command to obtain the 
	terminal's TTY.

2) Modify the "run.sh" script, replacing "pts/4" with the TTY obtained from 
	the newly opened terminal within the qemu command on lines 15-16. (Or, if 
	you're lucky, it'll already be pts/4, yay!)

3) Execute "run.sh" and follow the on screen prompts (simply pressing enter 
	works well) until asked to login. Currently, the following users and 
	passwords* are supported:

		login: root
		password: 12345

		login: kcw
		password: abcde

		login: guest
		password: mercy

		login: buster
		password: 123

	If you forget any of the logins, they will be displayed upon guessing 
	incorrectly, however due to a bug, incorrectly entering login credentials 
	will require the user to re-start the system by closing QEMU and re-running 
	the "run.sh" script.

* Note: It is acknowledged that displaying and storing passwords in plaintext 
	is a major security issue, and that in a system designed to run in earnest 
	(as opposed to teach operating systems principles), passwords would be 
	stored using cryptographic one-way hashes with salt, never unencrypted.

4) After logging in, the system may be used with major file system and file I/O
	commands. Further, the command "ps" may be used to view information on all 
	processes, and the user may use "logout" or "exit" to logout.


Currently known bugs and abnormalities:

- Incorrectly entering the login/password information prevents even correct 
	combinations from working until the program is restarted. It is suspected 
	there is some issue with not being able to search from the beginning of the 
	login credentials file on the second and onward attempts, possible due to 
	needing resetting/closing and reopening of the file.

- If the cat command is desired to be used for redirects, then "cat" may be 
	used, but due to not-yet-fully-known details regarding the common 
	implementation methods, this version does not work with the keyboard; 
	instead, to use cat with the keyboard, use "Cat" (with a capital 'C').

- For similar reasons as above, "more" only works with input redirection, and 
	not from the keyboard.