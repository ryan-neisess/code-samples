#! /bin/bash
rm mydisk
./kcmkfs.sh
gcc -m32 -g -o rmnfs rmnfs.c
./rmnfs mydisk
