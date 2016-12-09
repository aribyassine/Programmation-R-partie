#include "chat_common.h"

struct myshm *message;

int main(int argc, char const *argv[]) {
  if (argc != 2) {
    printf("Nombre d'argument incorrect\n");
    return EXIT_FAILURE;
  }
  char const *serveur_name = argv[1];
  int fd;
  int shm;
  /* Créé le segment en lecture écriture */
  if ((fd = shm_open(serveur_name, O_RDWR | O_CREAT, 0666)) == -1) {
    perror("shm_open ");
    exit(EXIT_FAILURE);
  }

  /* Allouer au segment une taille*/
  if (ftruncate(fd, sizeof(struct myshm)) == -1) {
    perror("ftruncate shm");
    exit(EXIT_FAILURE);
  }

  if ((message = mmap(NULL, sizeof(struct myshm), PROT_READ | PROT_WRITE,
                      MAP_SHARED, fd, 0) == MAP_FAILED)) {
    perror("mmap");
    exit(EXIT_FAILURE);
  }

  return EXIT_SUCCESS;
}
