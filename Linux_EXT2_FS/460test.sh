#! /bin/bash
rm 460diskcopy
cp 460disk 460diskcopy
gcc -m32 -g -o rmnfs rmnfs.c
./rmnfs 460diskcopy
