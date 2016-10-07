#define _XOPEN_SOURCE 700
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

int n = 1;
int nb_proc = 0;
int *pids;
/* La fonction create_process duplique le processus appelant et retourne
         le PID du processus fils ainsi créé */
pid_t create_process(void) {
  pid_t pid;
  do {
    pid = fork();
  } while ((pid == -1) && (errno == EAGAIN));
  return pid;
}

/* La fonction child_process effectue les actions du processus fils */
void child_process(void) {
  pids[n - 1] = getppid();
  if (n == nb_proc) {
    printf(
        "Une valeur aléatoire entre 0 et 100 : %d\nles pids des processus :\n",
        (int)(rand() / (((double)RAND_MAX + 1) / 100)));
    int i = 0;
    for (i = 0; i < nb_proc; i++) {
      printf("processus %d => %d | ", i + 1, pids[i]);
    }
    printf("processus : %d, pid :%d\n", nb_proc + 1, getpid());
  }
}

/* La fonction father_process effectue les actions du processus père */
void father_process(int child_pid) {

  if (wait(NULL) == -1) {
    perror("wait :");
    exit(EXIT_FAILURE);
  }
  printf("\n Nous sommes dans le père %d\n"
         " Le PID du fils est %d.\n"
         " Le PID du père est %d.\n",
         n, (int)child_pid, (int)getpid());
}

int main(int argc, char *argv[]) {
  if (argc != 2) {
    printf("Nombre d'argument incorrect\n");
    return EXIT_FAILURE;
  }
  nb_proc = atoi(argv[1]);
  pid_t pid;
  int tab[nb_proc];
  pids = tab;

  do {
    pid = create_process();
    switch (pid) {
    case -1:
      perror("fork");
      return EXIT_FAILURE;
      break;
    /* Si on est dans le fils */
    case 0:
      child_process();
      break;
    /* Si on est dans le père */
    default:
      father_process(pid);
      break;
    }
  } while ((pid == 0) && (++n <= nb_proc));
  return EXIT_SUCCESS;
}
