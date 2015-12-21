#include <sys/stat.h>
#include <fcntl.h>      // These two headers are for the file I/O syscalls
#include <ctype.h>
#include "../tlpi-dist/lib/tlpi_hdr.h"

int main(int argc, char *argv[])
{
  int fd, numBytes, flags;
  if (argc != 3 && argc != 4 )
    errExit("Usage: %s filename num-bytes [x]", argv[0]);
  numBytes = atoi(argv[2]);
  /* w/o x */
  if (argc == 3 )
    flags = O_RDWR | O_CREAT | O_APPEND;
  else
    flags = O_RDWR | O_CREAT;
  if (-1 == (fd = open(argv[1], flags, S_IRUSR | S_IWUSR)))
    errExit("open");
  for (size_t i = 0; i < numBytes; i++) {
    if (4 == argc)
      if (-1 == lseek(fd, 0, SEEK_END))
        errExit("lseek");
    if (-1 == write(fd, "a", 1))
      errExit("write");
  }
  exit(EXIT_SUCCESS);
}
