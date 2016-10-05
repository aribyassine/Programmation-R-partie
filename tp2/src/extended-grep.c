#define _XOPEN_SOUCE 700

#include <dirent.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

extern int errno;

#define _POSIX_SOUCE 1
#define TAILLE_PATH 100

char buff_path[TAILLE_PATH];
DIR *pt_Dir;
struct dirent *dirEnt;
struct stat stat_info;

void find_str(char *str, char *substr, char *file, int line, int *found) {
  char *pos = strstr(str, substr);
  if (pos) {
    printf("found the string '%s' in '%s' at position %d:%ld\n", substr, file,
           line, pos - str);
    *found = 1;
  }
}

int main(int argc, char *argv[]) {
  char *cible;
  if (argc == 1) {
    printf("Nombre d'arguments incorrects\n");
    return EXIT_FAILURE;
  } else if (argc == 2) {
    /* repertoir courant : obtenir le nom */
    cible = argv[2];
    if (getcwd(buff_path, TAILLE_PATH) == NULL) {
      perror("erreur getwcd \n");
      exit(1);
    }
  } else {
    cible = argv[1];
    memcpy(buff_path, argv[2], strlen(argv[2]));
  }
  if ((pt_Dir = opendir(buff_path)) == NULL) {
    if (errno == ENOENT) {
      /* repertoire n'existe pas - créer le répertoire */
      if (mkdir(buff_path, S_IRUSR | S_IWUSR | S_IXUSR) == -1) {
        perror("erreur mkdir\n");
        exit(1);
      } else
        return 0;
    } else {
      perror("erreur opendir \n");
      exit(1);
    }
  }
  chdir(buff_path);
  /* lire répertoire */
  int found = 0;
  while ((dirEnt = readdir(pt_Dir)) != NULL) {
    char *file_name = dirEnt->d_name;
    if (stat(file_name, &stat_info) == -1) {
      perror(file_name);
      return errno;
    }
    if (!S_ISDIR(stat_info.st_mode)) {
      FILE *fp = fopen(file_name, "r");
      if (!fp) {
        perror("File opening failed");
        return EXIT_FAILURE;
      }

      int file_size = stat_info.st_size;
      char buf[file_size];
      int line = 0;
      while (fgets(buf, sizeof buf, fp) != NULL) {
        find_str(buf, cible, file_name, ++line, &found);
      }
      if (ferror(fp))
        puts("I/O error when reading");
      fclose(fp);
    }
  }
  if (!found) {
    printf("the string '%s' was not found \n", cible);
  }
  closedir(pt_Dir);

  return 0;
}
