
#include <sys/stat.h>
#include <fcntl.h>      // These two headers are for the file I/O syscalls
#include <ctype.h>
#include "tlpi_hdr.h"

#define WRITE(BYTES)                        \
    numWritten = write(fd, buf, BYTES);     \
    if (numWritten == -1) errExit("write"); \

#define LSEEK(BYTES)                        \
    if (-1 == lseek(fd, BYTES, SEEK_CUR))   \
        errExit("lseek");                   \

#define PREPARE                                             \
    int fd;                                                 \
    size_t len = getBlkSize();                              \
    ssize_t numWritten;                                     \
    char *buf = (char*)malloc(len);                         \
    if (buf == NULL) errExit("malloc");                     \
    for (int i = 0 ; i < len ; i ++)                        \
        buf[i] = 's';                                       \

#define OPEN(filename)                                      \
    fd = open(filename, O_RDWR | O_CREAT | O_TRUNC,         \
                S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP |     \
                S_IROTH | S_IWOTH);                         \
    if (fd == -1) errExit("open");                          \


#define RESULT(ID)                                                                          \
    printf("Test %2d result: %s\n", ID, (mycp(argv[1], argv[2]) == 1) ? "PASS" : "FAIL" );  \

#define CLEAN       \
    free(buf);      \
    close(fd);      \

int getBlkSize();
void displayStatInfo(struct stat *sb, const char *filename);
int mycp(const char *src, const char *des);

 
int main(const int argc, const char **argv)
{
/*
 * argv[0]  program name ./4_2
 * argv[1]  src file
 * argv[2]  des file
 */
    /* create the input file with holes like this
     */
    PREPARE
    /* test 0 
     * file size = 8197 bytes
     * block size = 8192 bytes (two blocks)
        ----------------------------------------------------------
        | 5 data 4091 hole | 4096 bytes of hole | 5 data EOF     |
        ----------------------------------------------------------  
     */
    OPEN(argv[1])
    WRITE(5)    LSEEK(getBlkSize()*2-5) WRITE(5)
    RESULT(0)

    /* test 1 
     * file size = 12289 bytes
     * block size = 12288 bytes (3 blocks)
        -----------------------------------------------------------------------------
        | 5 data 4091 hole | 4096 bytes of hole | 5 data 4091 hole | 1 data EOF     |
        -----------------------------------------------------------------------------  
     */
    OPEN(argv[1])
    WRITE(5)    LSEEK(getBlkSize()*2-5) WRITE(5)    LSEEK(4091) WRITE(1)
    RESULT(1)

    /* test 2 
     * file size = 12289 bytes
     * block size = 16384 bytes (4 blocks)
        -----------------------------------------------------------------------------
        | 5 data 4091 hole | 5 data 4091 hole | 5 data 4091 hole | 1 data EOF     |
        -----------------------------------------------------------------------------  
     */
    OPEN(argv[1])
    WRITE(5) LSEEK(4091) WRITE(5)    LSEEK(4091) WRITE(5)    LSEEK(4091) WRITE(1)
    RESULT(2)

    /* test 3 
     * file size = 4097 bytes
     * block size = 8192 bytes (2 blocks)
        -------------------------------------
        | 5 data 4091 hole | 1 data EOF     |
        ------------------------------------- 
     */
    OPEN(argv[1])
    WRITE(5) LSEEK(4091) WRITE(1)
    RESULT(3)

    /* test 4 
     * file size = 1 bytes
     * block size = 4096 bytes (1 blocks)
        -------------------------------------
        | 4096 bytes of hole | 1 data EOF     |
        ------------------------------------- 
     */
    OPEN(argv[1])
    LSEEK(4096) WRITE(1)
    RESULT(4)

    CLEAN
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
int mycp(const char *src, const char *des)
{
    int fdi, fdo;
    struct stat sb1, sb2;
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
    if (fstat(fdi, &sb1) == -1) errExit("fstat");
    len = sb1.st_size; // size of input file
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
    
    // test result
    if (fstat(fdo, &sb2) == -1) errExit("fstat");
    if ((sb1.st_size == sb2.st_size) && (sb1.st_blocks == sb2.st_blocks))
        return 1;
    else{
        printf("1 size = %d\t1 blocks = %d\n", (int)sb1.st_size, (int)sb1.st_blocks);
        printf("2 size = %d\t2 blocks = %d\n", (int)sb2.st_size, (int)sb2.st_blocks);
        return 0;
    }

    free(buf);
    close(fdi);
    close(fdo);
}










