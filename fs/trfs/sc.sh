umount -t trfs /mnt/trfs/
lsmod
rmmod trfs
lsmod
make clean
make
insmod trfs.ko
lsmod
mount -t trfs -o tfile=/usr/src/hw2-cse506g15/fs/trfs/asmita /usr/src/hw2-cse506g15/trfs_mount /mnt/trfs
