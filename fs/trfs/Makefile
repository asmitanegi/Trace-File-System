TRFS_VERSION="0.1"

EXTRA_CFLAGS += -DTRFS_VERSION=\"$(TRFS_VERSION)\"
#EXTRA_CFLAGS += -DEXTRA_CREDIT=10

obj-m += trfs.o

trfs-y := dentry.o file.o inode.o main.o super.o lookup.o mmap.o

INC=/lib/modules/$(shell uname -r)/build/arch/x86/include
 
all: clean treplay trfs
 
treplay: treplay.c
	gcc -lz -Wall -I$(INC)/generated/uapi -I$(INC)/uapi treplay.c -o treplay
 
trfs:
	make -Wall -Werror -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules
	insmod trfs.ko
	mount -t trfs -o tfile=/tmp/tfile1.txt /usr/src/hw2-cse506g15/dir_test  /mnt/trfs
	mount -lt trfs
 
clean:
	umount /mnt/trfs
	rmmod trfs
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean
	rm -rf treplay
	rm -rf /tmp/tfile1.txt

trctl: trctl.c
	gcc -Wall -I$(INC)/generated/uapi -I$(INC)/uapi trctl.c -o trctl 	

restore_dirs:
	rm -rf /usr/src/hw2-cse506g15/dir_test /usr/src/hw2-cse506g15/dir_replica
	mkdir /usr/src/hw2-cse506g15/dir_test
	cp -r /usr/src/hw2-cse506g15/dir_test /usr/src/hw2-cse506g15/dir_replica

change_mount:
	umount /mnt/trfs
	mount -t trfs -o tfile=/tmp/tfile1.txt /usr/src/hw2-cse506g15/dir_replica  /mnt/trfs

sample: 
	gcc -Wall -I$(INC)/generated/uapi -I$(INC)/uapi sample_write.c -o write
	gcc -Wall -I$(INC)/generated/uapi -I$(INC)/uapi sample_read.c -o read
