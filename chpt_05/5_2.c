#include <sys/stat.h>
#include <fcntl.h>      // These two headers are for the file I/O syscalls
#include <ctype.h>
#include "../tlpi-dist/lib/tlpi_hdr.h"

int main(int argc, char *argv[])
{
  int fd;
  off_t offset;
  ssize_t numWritten;
  if (argc != 2 )
    errExit("Usage: %s filename", argv[0]);
  fd = open(argv[1], O_RDWR | O_APPEND);
  if (-1 == fd) errExit("open");
  if (-1 == (offset = lseek(fd, 0, SEEK_SET))) errExit("lseek");
  numWritten = write(fd, "SOMETHING NEW", 13);
  if (-1 == numWritten) errExit("write");
  printf("Offset = %ld\n", (long) offset);
  exit(EXIT_SUCCESS);
}
