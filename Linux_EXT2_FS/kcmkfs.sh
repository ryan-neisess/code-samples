#! /bin/bash
dd if=/dev/zero of=mydisk bs=1024 count=1440
sudo mkfs -b 1024 mydisk 1440
sudo mount -o loop mydisk /mnt
(cd /mnt; sudo mkdir dir1 dir2 dir1/dir3 ; sudo touch file1 dir2/file2 dir1/dir3/file3; ls -l)
sudo umount /mnt
