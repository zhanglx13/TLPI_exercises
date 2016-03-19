
#include <sys/stat.h>
#include <fcntl.h>      // These two headers are for the file I/O syscalls
#include <ctype.h>
#include "tlpi_hdr.h"


int getBlkSize();
void displayStatInfo(struct stat *sb, const char *filename);
void mycp(const char *src, const char *des);


int main(const int argc, const char **argv)
{
/*
 * argv[0]  program name ./4_2
 * argv[1]  src file
 * argv[2]  des file
 */
    struct stat sb;
    int block_size = getBlkSize();
    printf("block size of fs: %d\n", block_size);

    /* create the input file with holes like this
    ----------------------------------------------------------------
    | 4096 bytes of data | 4096 bytes of hole | 4096 bytes of data |
    ----------------------------------------------------------------
     */
    int fd;
    size_t len = block_size;
    ssize_t numWritten;
    char *buf = (char *)malloc(len);
    if (buf == NULL) errExit("malloc");
    // open file
    fd = open(argv[1], O_RDWR | O_CREAT | O_TRUNC, 
                S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | 
                S_IROTH | S_IWOTH);     /* rw-rw-rw- */
    if(fd == -1) errExit("open");
    // initialize buf
    for (unsigned int i = 0; i < len ; i ++)
        buf[i] = 's';
    // write to file
    numWritten = write(fd, buf, 5);
    if (numWritten == -1) errExit("write");
    // seek beyond the hole in the file
    if (lseek(fd, block_size*2-5, SEEK_END) == -1)
        errExit("lseek");
    // write to file again
    numWritten = write(fd, buf, 5);
    if (numWritten == -1) errExit("write");


    // copy file including holes
    mycp(argv[1], argv[2]);

    // get stat of input and output file
    if (stat(argv[1], &sb) == -1) errExit("stat");
    displayStatInfo(&sb, argv[1]);
    
    if (stat(argv[2], &sb) == -1) errExit("stat");
    displayStatInfo(&sb, argv[2]);
    
    free(buf);
    close(fd);
    exit(EXIT_SUCCESS);

}

/*
 * Get block size of the file system at runtime
 */
int getBlkSize()
{
    struct stat sb;
    int fd;
    ssize_t numWritten;
    char buf = 's';
    // open file
    fd = open("dummy", O_RDWR | O_CREAT | O_TRUNC,
                S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP |
                S_IROTH | S_IWOTH);     /* rw-rw-rw- */
    if(fd == -1) errExit("open");
    // write to file
    numWritten = write(fd, &buf, 1);
    if (numWritten == -1) errExit("write");
    // get stat of input file
    if (fstat(fd, &sb) == -1) errExit("fstat");
    close(fd);
    remove("dummy");
    return sb.st_blocks * 512;
}


void displayStatInfo(struct stat *sb, const char *filename)
{
    printf("Stat info for %s:\n", filename);
    printf("size: %lld bytes\n", (long long)sb->st_size);
    printf("Allocated size on disk: %lld bytes\n", (long long)sb->st_blocks * (long long)512);
}

/* 
 * Copy file src to file des.
 * If src has holes, also create holes in des
 */
void mycp(const char *src, const char *des)
{
    int fdi, fdo;
    struct stat sb;
    unsigned int len;
    int buf_len = getBlkSize();
    char ch;
    char *buf = (char*)malloc(buf_len);
    if (buf == NULL) errExit("malloc");
    fdi = open(src, O_RDONLY, 
                S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP |
                S_IROTH | S_IWOTH);     /* rw-rw-rw- */
    fdo = open(des, O_WRONLY | O_TRUNC, /* remember to truncate the output file when opening */
                S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP |
                S_IROTH | S_IWOTH);     /* rw-rw-rw- */
    if (fstat(fdi, &sb) == -1) errExit("fstat");
    len = sb.st_size; // size of input file
    unsigned int offset = 0;
    ssize_t numRead, numWritten;
    // loop for each byte in the input file 
    for (offset = 0 ; offset < len ; offset ++){
        numRead = read(fdi, &ch, 1);    // read one byte a time
        if (numRead == -1) errExit("read");
        // hole detected
        if (ch == NULL){ 
            if (-1 == lseek(fdo, 1, SEEK_CUR))  // increment the offset of the output file
                errExit("lseek"); 
        }
        // data detected
        else{
            numWritten = write(fdo, &ch, 1);
            if (numWritten == -1) errExit("write");
        }
    }
    free(buf);
    close(fdi);
    close(fdo);
}










