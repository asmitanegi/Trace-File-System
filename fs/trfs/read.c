#include <stdio.h>
#include <errno.h>
int main (void) {
    FILE *fp;
    char str[] = "THIS IS ASMITA NEGI.i I STAY IN STONY BROOK I STUDY IN STONY BROOK UNIVERSITY ITS IS COLD HERE BUT I LIKE IT";
    fp = fopen ("/mnt/trfs/asm/ggg/file1","a");
    if (fp == NULL) {
        printf ("File not created okay, errno = %d\n", errno);
        return 1;
    }
    fwrite(str , 1 , strlen(str) , fp );
    //fprintf (fp, "Hello, there.\n"); // if you want something in the file.
    fclose (fp);
    printf ("File created okay\n");
    return 0;
}
