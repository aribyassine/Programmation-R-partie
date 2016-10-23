#define _XOPEN_SOURCE 700
#define _REENTRANT
#include "lib_upper.h"
#include <errno.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int index = 0;
int nb_file;
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

void *func_thread(void *arg) {
  int param;
  int upper_result;
  char **argv = (char **)arg;

  while (index < nb_file) {
    pthread_mutex_lock(&mutex);
    index++;
    param = index + 1;
    pthread_mutex_unlock(&mutex);
    printf("thread %d %s\n", (int)pthread_self(), argv[param]);
    if (upper(argv[param]) != 0) {
      pthread_exit((void *)param);
    }
  }
  pthread_exit((void *)0);
}

int main(int argc, char *argv[]) {
  if (argc < 3) {
    printf("Nombre d'argument incorrect\n");
    return EXIT_FAILURE;
  }
  nb_file = argc - 2;
  int nb_threads = atoi(argv[1]);

  pthread_t tid[nb_threads];
  int i, status[nb_threads];

  for (i = 0; i < nb_threads; i++) {

    if (pthread_create(&(tid[i]), NULL, func_thread, (void *)argv) != 0) {
      perror("pthread_create");
      return EXIT_FAILURE;
    }
  }
  int nb_error = 0;
  for (i = 0; i < nb_threads; i++) {
    if (pthread_join(tid[i], (void **)&status[i]) != 0) {
      perror("error pthread_join");
      return EXIT_FAILURE;
    }
    if (status[i] != 0) {
      printf("%s\n", argv[(int)status[i]]);
      nb_error++;
    }
  }
  return nb_error;
}
