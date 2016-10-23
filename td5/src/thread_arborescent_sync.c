#define _XOPEN_SOURCE 700
#define _REENTRANT
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

int max;
int total = 0;

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t condition = PTHREAD_COND_INITIALIZER;
int nbsignal(int lvl) {
  int result = 1;
  for (size_t i = 1; i <= lvl; i++) {
    result *= i;
  }
  return result - 1;
}
int formule(int lvl) {
  int result = 0;
  int x;
  lvl += 2;
  for (int i = 2; i <= lvl; i++) {
    x = 1;
    for (int j = 1; j < i; j++) {
      x = x * j;
    }
    result += x;
  }
  return result - 1;
}

void *thread_func(void *arg) {
  int i, nb;
  int *param;
  int lvl = (int)arg;
  pthread_t *tid;

  nb = lvl + 1;

  if (lvl < max) {
    param = (int *)malloc(sizeof(int));
    *param = nb;
    tid = calloc(nb, sizeof(pthread_t));
    printf("%d cree %d fils au niveau %d\n", (int)pthread_self(), nb, lvl);
    total += nb;

    if (total == formule(lvl)) {
      for (size_t i = 0; i < nbsignal(lvl); i++) {
        pthread_mutex_lock(&mutex);
        pthread_cond_signal(&condition);
        pthread_mutex_unlock(&mutex);
      }
      printf("===== fin du niveau %d =====\n", lvl);
      printf("total %d\n", total);
    } else {
      pthread_mutex_lock(&mutex);
      pthread_cond_wait(&condition, &mutex);
      pthread_mutex_unlock(&mutex);
    }

    for (i = 0; i < nb; i++) {
      pthread_create((tid + i), 0, thread_func, (void *)*param);
    }
    for (i = 0; i < nb; i++)
      pthread_join(tid[i], NULL);
  }

  if (lvl > 1)
    pthread_exit((void *)0);

  return (void *)0;
}

int main(int argc, char *argv[]) {

  if (argc != 2) {
    printf("Nombre d'argument incorrect\n");
    return EXIT_FAILURE;
  }
  int nb_threads = 1;
  max = atoi(argv[1]);
  pthread_t tid;

  if (pthread_create(&tid, NULL, thread_func, (void *)nb_threads) != 0) {
    perror("pthread_create");
    return EXIT_FAILURE;
  }
  pthread_join(tid, NULL);
  printf("total : %d\n", total);
}
