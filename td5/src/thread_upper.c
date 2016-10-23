#define _XOPEN_SOURCE 700
#define _REENTRANT
#include "lib_upper.h"
#include <errno.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

void *func_thread(void *arg) { pthread_exit((void *)upper((char *)arg)); }

int main(int argc, char *argv[]) {
  if (argc < 2) {
    printf("Nombre d'argument incorrect\n");
    return EXIT_FAILURE;
  }
  int nb_threads = argc - 1;
  pthread_t tid[nb_threads];
  int i, status[nb_threads];

  for (i = 0; i < nb_threads; i++) {

    if (pthread_create(&(tid[i]), NULL, func_thread, (void *)argv[i + 1]) !=
        0) {
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
      printf("%s\n", argv[i + 1]);
      nb_error++;
    }
  }
  return nb_error;
}
