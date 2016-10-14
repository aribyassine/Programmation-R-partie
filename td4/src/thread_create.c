#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

void *func_thread(void *arg) {
  int num = (int)arg;
  printf("Argument reçu %d, tid: %d\n", num, (int)pthread_self());
  pthread_exit((void *)(num * 2));
}

int main(int argc, char *argv[]) {
  if (argc != 2) {
    printf("Nombre d'argument incorrect\n");
    return EXIT_FAILURE;
  }
  int nb_threads = atoi(argv[1]);
  int i, status[nb_threads];
  pthread_t tid[nb_threads];
  for (i = 0; i < nb_threads; i++) {
    if (pthread_create(&(tid[i]), NULL, func_thread, (void *)i) != 0) {
      perror("pthread_create");
      return EXIT_FAILURE;
    }
  }
  for (i = 0; i < nb_threads; i++) {
    if (pthread_join(tid[i], (void **)&status[i]) != 0) {
      perror("error pthread_join");
      return EXIT_FAILURE;

    } else
      printf("Thread %d fini avec status :%d\n", i, status[i]);
  }
  return EXIT_SUCCESS;
}
