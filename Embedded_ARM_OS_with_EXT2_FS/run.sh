rm sdimage
cp sdimage_original sdimage

sudo ./mk.sh init
sudo ./mk.sh login
sudo ./mk.sh sh
sudo ./mk.sh ls
sudo ./mk.sh cat
sudo ./mk.sh Cat
sudo ./mk.sh more
sudo ./mk.sh l2u
sudo ./mk.sh grep
sudo ./mk.sh cp

qemu-system-arm -M versatilepb -m 128M -sd sdimage -kernel wanix \
-serial mon:stdio -serial /dev/pts/4 #-serial /dev/pts/6 #-serial /dev/pts/6

# qemu-system-arm -M versatilepb -m 128M -sd sdimage -kernel wanix \
# -serial mon:stdio 

# qemu-system-arm -M versatilepb -m 128M -sd sdimage -kernel wanix \
# -serial mon:stdio 