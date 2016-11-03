#define _XOPEN_SOURCE 700
#define _REENTRANT
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main(int argc, char *argv[]) {
  char buf;
  if (argc != 2) {
    printf("Nombre d'argument incorrect\n");
    return EXIT_FAILURE;
  }

  int fd;
  fd = open(argv[1], O_WRONLY);

  while (read(STDOUT_FILENO, &buf, 1) > 0) {
    write(fd, &buf, 1);
  }
}
