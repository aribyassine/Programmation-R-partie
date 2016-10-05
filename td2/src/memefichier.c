#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>

int main(int argc, char *argv[]) {
  struct stat stat_info1;
  struct stat stat_info2;

  if (stat(argv[1], &stat_info1) == -1) {
    perror(argv[1]);
    return errno;
  }
  if (stat(argv[2], &stat_info2) == -1) {
    perror(argv[2]);
    return errno;
  }

  if (stat_info1.st_ino == stat_info2.st_ino &&
      stat_info1.st_dev == stat_info2.st_dev) {
    printf("meme fichier\n");
  } else {
    printf("pas le meme fichier\n");
  }
  return EXIT_SUCCESS;
}
