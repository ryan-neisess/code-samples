#! /bin/bash
rm testdisk
#./kcmkfs.sh
cp diskimage testdisk
gcc -m32 -g -o showblock showblock.c
./showblock testdisk /Y/bigfile