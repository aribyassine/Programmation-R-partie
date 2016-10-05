#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

int main(int argc, char *argv[]) {
  struct stat stat_info;
  if (stat(argv[2], &stat_info) == -1) {
    perror(argv[2]);
    return errno;
  }
  if (S_ISDIR(stat_info.st_mode)) {
    printf("%s est un repertoire\n", argv[2]);
    return EXIT_FAILURE;
  }

  switch (argv[1][0]) {
  case 'e':
  case 'E':
    if (unlink(argv[2]) == -1) {
      perror(argv[2]);
      return errno;
    }
    if (stat_info.st_nlink == 0) {
      printf("%s supprimé\n", argv[2]);
    }
    break;

  case 'R':
  case 'r':
    if (rename(argv[2], argv[3]) == -1) {
      perror(argv[3]);
      return errno;
    }
    break;

  case 'C':
  case 'c':
    if (argv[3][0] == 'R' || argv[3][0] == 'r') {
      if (chmod(argv[2], S_IRUSR | S_IRGRP | S_IROTH | S_IWUSR) == -1) {
        perror(argv[3]);
        return errno;
      }
    } else if (argv[3][0] == 'W' || argv[3][0] == 'w') {
      if (chmod(argv[2], S_IWUSR | S_IWGRP | S_IWOTH | S_IRUSR) == -1) {
        perror(argv[3]);
        return errno;
      }
    } else {
      printf("%s n'est pas une option valide \n", argv[3]);
      return EXIT_FAILURE;
    }

    break;

  default:
    printf("%s n'est pas une option valide \n", argv[1]);
    return EXIT_FAILURE;
    break;
  }

  return EXIT_SUCCESS;
}
