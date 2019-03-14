#! /bin/bash
rm mydisk
./kcmkfs.sh
#gcc -m32 -g -o mount_root mount_root.c util.o cd_ls_pwd.o
gcc -m32 -g -o mount_root mount_root.c util.o
./mount_root mydisk