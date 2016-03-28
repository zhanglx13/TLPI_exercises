/*************************************************************************\
*                  Copyright (C) Michael Kerrisk, 2015.                   *
*                                                                         *
* This program is free software. You may use, modify, and redistribute it *
* under the terms of the GNU General Public License as published by the   *
* Free Software Foundation, either version 3 or (at your option) any      *
* later version. This program is distributed without any warranty.  See   *
* the file COPYING.gpl-v3 for details.                                    *
\*************************************************************************/

/* Exercise 5-7 */

/* 5_7.c

   Implementation of readv() and writev() using read(), write(), 
   and malloc()'s.

   The program first generates a source file 5_7_src.txt which contains
   stats of file temp.txt, integer x, and a string of 100 chars.

   Then the readv() and writev() functions are tested using this file.

*/
#include <sys/stat.h>
#include <sys/uio.h>
#include <fcntl.h>
#include "tlpi_hdr.h"

#define STR_SIZE 26

void genFile(char **argv)
{
    int fd;
    struct iovec iov[3];
    struct stat file_stat;  // first buffer
    int x = 5;              // second buffer
    char str[STR_SIZE];     // third buffer
    ssize_t numWritten, totRequired;

    fd = open("temp.txt", O_RDONLY);
    if (-1 == fd) errExit("open");

    if (-1 == fstat(fd, &file_stat)) errExit("fstat");
    close(fd);
    for (unsigned int i = 0 ; i < STR_SIZE ; i ++){
        str[i] = 'a' + i;
    }
    str[STR_SIZE] = '\0';
    printf("str = %s\n", str);

    iov[0].iov_base = &file_stat;
    iov[0].iov_len = sizeof(struct stat);
    totRequired = iov[0].iov_len;

    iov[1].iov_base = &x;
    iov[1].iov_len = sizeof(int);
    totRequired += iov[1].iov_len;

    iov[2].iov_base = str;
    iov[2].iov_len = STR_SIZE;
    totRequired += iov[2].iov_len;
    
    fd = open(argv[1], O_RDWR | O_TRUNC);
    numWritten = writev(fd, iov, 3);
    if (-1 == numWritten) errExit("writev");
    if (numWritten < totRequired) printf("Written fewer bytes than requested!\n");
    printf("Total bytes requested: %ld; bytes written: %ld\n", totRequired, numWritten);

    close(fd);
//    exit(EXIT_SUCCESS);
}

ssize_t my_readv(int fd, struct iovec *iov, int iovcnt)
{
    if (iovcnt <= 0){
        printf("Required nothing to read\n");
        return 0;
    }
    int totLen = 0;
    ssize_t numRead;
    for (unsigned int i = 0 ; i < iovcnt ; i ++)
        totLen += iov[i].iov_len;
    /* Read the data from file into the buffer */
    char *buf = (char*)malloc(totLen);
    numRead = read(fd, buf, totLen);
    if (-1 == numRead) errExit("read");
    for (unsigned int i = 0 ; i < totLen ; i ++)
        printf("%c",buf[i]);
    printf("\n");
    /* move data from the buffer into iovec structs */
    strcpy(iov[0].iov_base, buf);   //  Assume strcpy performs deep copy
    printf("iov0: %d\n", iov[0].iov_len);
    for (unsigned int i = 0 ; i < iov[0].iov_len ; i ++)
        printf("%c", ((char*)iov[0].iov_base+i));
    printf("\n");
    for (unsigned int i = 1 ; i < iovcnt ; i ++){
        strcpy(iov[i].iov_base, (char*)iov[i-1].iov_base+iov[i-1].iov_len);
        printf("iov%d: %ld\n", i, (long)iov[i].iov_len);
        for (unsigned int j = 0 ; j < iov[i].iov_len ; j ++)
            printf("%c", *((char*)iov[i].iov_base+j));
        printf("\n");
    }
    /* clean up */
    free(buf);
    /* return the number of bytes read from the file */
    return numRead;

}

ssize_t my_writev(int fd, const struct iovec *iov, int iovcnt)
{
    
}

/*
   Compare if two iovec struct contains the exactly same bytes of data
 */
Boolean isEqu(const struct iovec *iov1, const struct iovec *iov2, int iovcnt)
{
    for (int i = 0 ; i < iovcnt ; i ++){
        if ( (iov1[i].iov_len != iov2[i].iov_len))
            return FALSE;
        for (int j = 0 ; j < iov1[i].iov_len ; j ++)
            //if ( (char)iov1[i].iov_base[j] != (char)iov2[i].iov_base[j])
                return FALSE;
    }
    return TRUE;
}

int main(int argc, char *argv[])
{
    genFile(argv);
    int fd;
    struct iovec iov[3];
    struct iovec iov1[3];
    struct stat myStruct;       /* First buffer */
    int x;                      /* Second buffer */
    char str[STR_SIZE];         /* Third buffer */
    ssize_t numRead, totRequired;

    if (argc != 2 || strcmp(argv[1], "--help") == 0)
        usageErr("%s file\n", argv[0]);

    fd = open(argv[1], O_RDONLY);
    if (fd == -1)
        errExit("open");

    totRequired = 0;

    iov[0].iov_base = &myStruct;
    iov[0].iov_len = sizeof(struct stat);
    totRequired += iov[0].iov_len;

    iov[1].iov_base = &x;
    iov[1].iov_len = sizeof(x);
    totRequired += iov[1].iov_len;

    iov[2].iov_base = str;
    iov[2].iov_len = STR_SIZE;
    totRequired += iov[2].iov_len;
    /* Allocate space for iovec's for testing my_readv function */
    iov1[0].iov_base = (void*)malloc(iov[0].iov_len);
    iov1[0].iov_len = iov[0].iov_len;
    iov1[1].iov_base = (void*)malloc(iov[1].iov_len);
    iov1[1].iov_len = iov[1].iov_len;
    iov1[2].iov_base = (void*)malloc(iov[2].iov_len);
    iov1[2].iov_len = iov[2].iov_len;

    numRead = readv(fd, iov, 3);
    if (numRead == -1)
        errExit("readv");
    if (numRead < totRequired)
        printf("Read fewer bytes than requested\n");

    printf("iov0: %d\n", iov[0].iov_len);
    for (unsigned int i = 0 ; i < iov[0].iov_len ; i ++)
        printf("%c", ((char*)iov[0].iov_base+i));
    printf("\n");
    for (unsigned int i = 1 ; i < 3 ; i ++){
        printf("iov%d: %ld\n", i, (long)iov[i].iov_len);
        for (unsigned int j = 0 ; j < iov[i].iov_len ; j ++)
            printf("%c", *((char*)iov[i].iov_base+j));
        printf("\n");
    }




    printf("total bytes requested: %ld; bytes read: %ld\n",
            (long) totRequired, (long) numRead);
    /* Test my_readv() function */
    if (-1 == lseek(fd, 0, SEEK_SET)) errExit("lseek");
    numRead = my_readv(fd, iov1, 3);
    if (numRead == -1)
        errExit("readv");
    if (numRead < totRequired)
        printf("Read fewer bytes than requested\n");
    printf("total bytes requested: %ld; bytes read: %ld\n",
            (long) totRequired, (long) numRead);
    
    printf(isEqu(iov, iov1, 3) ? "PASS\n" : "FAIL\n");

    exit(EXIT_SUCCESS);
}
