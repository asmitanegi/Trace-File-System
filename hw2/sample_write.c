#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include<stdlib.h>
int main (void) {
    int fp;
    ssize_t bytes_written=0;
    //str = (char *)malloc(200);
    //strcpy(str,"THIS IS ASMITA NEGI.");
    char* str = "THIS IS A TEST FILE";
    fp = open ("/mnt/trfs/testNew.txt",O_WRONLY|O_APPEND|O_CREAT,0777);
    if (fp<0) {
        printf("File not created okay, errno = %d\n", errno);
        return 1;
    }
    printf("File open successful\n");
    
    bytes_written = write(fp,str,30);
    if (bytes_written <=0)
	return 1;
    printf("File write successful\n");   
    close(fp);
    printf("File close successful\n");
    
   return 0;
}  
