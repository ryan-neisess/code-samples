#! /bin/bash
rm lv2testdisk
cp lv2disk lv2testdisk
gcc -m32 -g -o rmnfs rmnfs.c
./rmnfs lv2testdisk
