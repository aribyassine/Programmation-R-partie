#define _XOPEN_SOURCE 700
#define _REENTRANT
#include <ctype.h>
#include <fcntl.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
char *arg;
int fd;

void sig_hand(int sig) {
  remove(arg);
  close(fd);
  return EXIT_SUCCESS;
}

int main(int argc, char *argv[]) {
  char buf;
  if (argc != 2) {
    printf("Nombre d'argument incorrect\n");
    return EXIT_FAILURE;
  }
  arg = argv[1];
  if (mkfifo(argv[1], 0666) != 0) {
    perror("mkfifo");
    return EXIT_FAILURE;
  }
  signal(SIGINT, sig_hand);
  fd = open(argv[1], O_RDONLY);
  while (read(fd, &buf, 1) > 0) {
    buf = toupper(buf);
    write(STDOUT_FILENO, &buf, 1);
  }
  remove(arg);
  close(fd);
  return EXIT_SUCCESS;
}
