
#include <sys/stat.h>
#include <fcntl.h>      // These two headers are for the file I/O syscalls
#include <ctype.h>
#include "tlpi_hdr.h"

#define MAX_READ 20
#define printable(ch) (isprint((unsigned char) ch) ? ch : '#')

static void usageError(char *progName, char *msg, int opt)
{
  if(msg != NULL && opt != 0)
    fprintf(stderr, "%s (-%c)\n", msg, printable(opt));
  fprintf(stderr, "Usage: %s [-a] filename\n", progName);
  exit(EXIT_FAILURE);
}

void mytee(char *filename, int flags)
{
  int fd;
  size_t len = MAX_READ;
  ssize_t numRead, numWritten;
  char buf[MAX_READ];
  if ((fd = open(filename, flags, S_IRUSR | S_IWUSR)) == -1) errExit("open");
  while((numRead = read(0, buf, len)) > 0){ /* Read from standard input fd = 0 */
    /* Write to standard output fd = 1 */
    //printf("numRead = %ld\n", (long)numRead);
    if (-1 == (numWritten = write(1, buf, numRead))) errExit("write1");
    /* Write to file */
    if (-1 == (numWritten = write(fd, buf, numRead))) errExit("write2");
  }
  if (-1 == numRead) errExit("read");
  if (0 == numRead)
    if (-1 == (numWritten = write(1, "\nCtrl-D pressed!", 16))) errExit("write1");

  if (-1 == close(fd)) errExit("close");
}

int main(int argc, char *argv[])
{
  int opt;
  char *pstr;

  /* For this exercise, we restrict the command line to be
   * either ./prog -a filename or ./prog filename.
   * no multiple options are allowed.
   */
  if (argc != 2 && argc != 3)
    usageError(argv[0], NULL, 0);

  opt = getopt(argc, argv, ":a:");
  switch (opt) {
    case 'a':
      pstr = optarg;
      mytee(pstr, O_RDWR | O_CREAT | O_APPEND);
      break;
    case ':':
      usageError(argv[0], "Missing argument", optopt);
      break;
    case '?':
      usageError(argv[0], "Unrecognized option", optopt);
      break;
    case -1: /* no option found */
      if (argc != 2) usageError(argv[0], NULL, 0);
      mytee(argv[1], O_RDWR | O_CREAT | O_TRUNC);
      break;
  }
  exit(EXIT_SUCCESS);
}
