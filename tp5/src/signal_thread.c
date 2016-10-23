#define _XOPEN_SOURCE 700
#define _REENTRANT
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t condition = PTHREAD_COND_INITIALIZER;

void sig_hand(int sig) { printf("signal reçu %d \n", sig); }
void *func_thread(void *arg) {

  sigset_t mask;
  sigfillset(&mask);
  sigprocmask(SIG_SETMASK, &mask, NULL);

  int nb_threads = ((int)arg) - 1;
  if (nb_threads > 0) {
    pthread_t tid;
    if (pthread_create(&tid, NULL, func_thread, (void *)nb_threads) != 0) {
      perror("pthread_create");
      return EXIT_FAILURE;
    }
    pthread_join(tid, NULL);
  } else {
    pthread_mutex_lock(&mutex);
    pthread_cond_wait(&condition, &mutex);
    pthread_mutex_unlock(&mutex);
  }
  printf("fin thread no %d\n", nb_threads);
}

int main(int argc, char *argv[]) {
  if (argc != 2) {
    printf("Nombre d'argument incorrect\n");
    return EXIT_FAILURE;
  }
  int nb_threads = atoi(argv[1]);
  pthread_t tid;
  if (pthread_create(&tid, NULL, func_thread, (void *)nb_threads) != 0) {
    perror("pthread_create");
    return EXIT_FAILURE;
  }

  sigset_t sig_proc;
  struct sigaction action;
  sigemptyset(&sig_proc);

  /* changer le traitement */
  action.sa_mask = sig_proc;
  action.sa_flags = 0;
  action.sa_handler = sig_hand;
  sigaction(SIGINT, &action, NULL);

  /* attendre le signal SIGINT */
  sigfillset(&sig_proc);
  sigdelset(&sig_proc, SIGINT);
  sigsuspend(&sig_proc);

  pthread_mutex_lock(&mutex);
  pthread_cond_signal(&condition);
  pthread_mutex_unlock(&mutex);

  pthread_join(tid, NULL);
  printf("Tous mes descendants se sont terminés\n");
}
