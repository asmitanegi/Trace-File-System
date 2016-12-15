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
    char *str;
    str = (char*) malloc(sizeof(char)*200);
    ssize_t bytes_read=0;
    fp = open ("/mnt/trfs/testNew.txt",O_RDONLY);
    if (fp<0) {
        printf ("File not created  okay, errno = %d\n", errno);
        return 1;
    }
    printf("File open successful\n");

    bytes_read = read(fp,str,100);
    if (bytes_read <=0)
        return 1;
    printf("File read successful\n");
    printf("%s\n",str);
    close(fp);
    printf("File close successful\n");

   return 0;
}
