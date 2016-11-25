#define _XOPEN_SOURCE 700
#define _REENTRANT
#include "producteur_consommateur.h"
#include <errno.h>
#include <fcntl.h> /* For O_* constants */
#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <sys/stat.h> /* For mode constants */
#include <unistd.h>

#define PROD 0
#define CONS 1
#define buf_size 100

struct myshm {
  sem_t sem;
  sem_t mutex;
  char *buf[buf_size];
  int stack_size;
};

struct myshm *shm;

push(char c) {
  sem_wait(&(shm->mutex));
  shm->buf[shm->stack_size] = c;
  shm->stack_size++;
  sem_post(&(shm->mutex));
  sem_post(&(shm->sem));
  /*printf("\npush %d, %c\n", shm->stack_size - 1, c);*/
}
char pop() {
  sem_wait(&(shm->sem));
  sem_wait(&(shm->mutex));
  shm->stack_size--;
  char c = shm->buf[shm->stack_size];
  sem_post(&(shm->mutex));
  /*printf("\npop %d, %c\n", shm->stack_size + 1, c);*/
  return c;
}

/* La fonction child_process effectue les actions du processus fils */
void child_process(int type) {
  int buf;
  int shm_id;
  if (type == PROD) {
    PRODUCTEUR
  } else if (type == CONS) {
    CONSOMMATEUR
  }
  /*printf("%d\n", type);*/
  exit(0);
}
/* La fonction create_process duplique le processus appelant et retourne
         le PID du processus fils ainsi créé */
pid_t create_process() {
  pid_t pid;
  do {
    pid = fork();
  } while ((pid == -1) && (errno == EAGAIN));
  return pid;
}

int *create_n_process(int nb_proc, int type) {
  pid_t pid;
  int n = 0;
  int pids[nb_proc];
  do {
    pid = create_process();
    switch (pid) {
    case -1:
      perror("fork");
      return EXIT_FAILURE;
      break;
    /* Si on est dans le fils */
    case 0:
      child_process(type);
      break;
    default:
      pids[n] = pid;
      break;
    }
  } while ((pid != 0) && (++n < nb_proc));
  return pids;
}

int main(int argc, char *argv[]) {
  if (argc != 3) {
    printf("Nombre d'argument incorrect\n");
    return EXIT_FAILURE;
  }
  int nb_prod = atoi(argv[1]);
  int nb_cons = atoi(argv[2]);

  int shm_id;
  /* Créé le segment en lecture écriture */
  if ((shm_id = shm_open("/shm", O_RDWR | O_CREAT, 0666)) == -1) {
    perror("shm_open ");
    exit(EXIT_FAILURE);
  }

  /* Allouer au segment une taille*/
  if (ftruncate(shm_id, sizeof(struct myshm)) == -1) {
    perror("ftruncate shm");
    exit(EXIT_FAILURE);
  }
  if ((shm = mmap(NULL, sizeof(struct myshm), PROT_READ | PROT_WRITE,
                  MAP_SHARED, shm_id, 0)) == MAP_FAILED) {
    perror("mmap");
    exit(EXIT_FAILURE);
  }
  if (sem_init(&(shm->sem), 1, 0) == -1) {
    perror("sem_init ");
    exit(EXIT_FAILURE);
  }
  if (sem_init(&(shm->mutex), 1, 1) == -1) {
    perror("sem_init ");
    exit(EXIT_FAILURE);
  }

  create_n_process(nb_cons, CONS);
  create_n_process(nb_prod, PROD);

  /* “detacher” le segment */
  munmap(shm, sizeof(struct myshm));
  /* detruire le segment */
  shm_unlink("/shm");
  return EXIT_SUCCESS;
}
