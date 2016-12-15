all: treplay trctl sample

treplay: treplay.c
	gcc -lz -Wall -I$(INC)/generated/uapi -I$(INC)/uapi treplay.c -o treplay
 
trctl: trctl.c
	gcc -Wall -I$(INC)/generated/uapi -I$(INC)/uapi trctl.c -o trctl -D EXTRA_CREDIT=1       

sample: 
	gcc -Wall -I$(INC)/generated/uapi -I$(INC)/uapi sample_write.c -o write
	gcc -Wall -I$(INC)/generated/uapi -I$(INC)/uapi sample_read.c -o read


