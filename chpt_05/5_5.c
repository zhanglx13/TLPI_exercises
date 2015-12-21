#include <sys/stat.h>
#include <fcntl.h>      // These two headers are for the file I/O syscalls
#include <ctype.h>
#include "../tlpi-dist/lib/tlpi_hdr.h"

int main(int argc, char *argv[])
{
  int fd, newfd;
  off_t off, newoff;
  int flags, newflags;
  if (-1 == (fd = open(argv[1], O_RDWR | O_APPEND)))
    errExit("open");
  newfd = dup(fd);

  if (-1 == (off = lseek(fd, -10, SEEK_END)))
    errExit("lseek");
  newoff = lseek(newfd, 0, SEEK_CUR);
  if (-1 == newoff) errExit("lseek");
  flags = fcntl(fd, F_GETFL);
  if (-1 == flags) errExit("fcntl");
  newflags = fcntl(newfd, F_GETFL);
  if (-1 == newflags) errExit("fcntl");

  printf("file %d off %ld flags %d\n", fd, (long)off, flags);
  printf("file %d off %ld flags %d\n", newfd, (long)newoff, newflags);

  if (-1 == close(fd)) errExit("close");
  if (-1 == close(newfd)) errExit("close");

  exit(EXIT_SUCCESS);
}
