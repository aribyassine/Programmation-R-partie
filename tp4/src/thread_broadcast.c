#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t condition = PTHREAD_COND_INITIALIZER;

void wait_barrier(int arg) {
  static int nb_appel = 0;
  pthread_mutex_lock(&mutex);
  nb_appel++;
  pthread_mutex_unlock(&mutex);
  if (nb_appel == arg) {
    pthread_mutex_lock(&mutex);
    printf("=================================\n");
    pthread_cond_signal(&condition);
    pthread_mutex_unlock(&mutex);
  }
  pthread_mutex_lock(&mutex);
  pthread_cond_wait(&condition, &mutex);
  pthread_cond_signal(&condition);
  pthread_mutex_unlock(&mutex);
}
void *func_thread(void *arg) {
  printf("thread %d: avant barriere\n", (int)pthread_self());
  wait_barrier((int)arg);
  printf("thread %d: après barriere\n", (int)pthread_self());
  pthread_exit(NULL);
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
    if (pthread_create(&(tid[i]), NULL, func_thread, (void *)nb_threads) != 0) {
      perror("pthread_create");
      return EXIT_FAILURE;
    }
  }
  for (i = 0; i < nb_threads; i++) {
    if (pthread_join(tid[i], (void **)&status[i]) != 0) {
      perror("error pthread_join");
      return EXIT_FAILURE;
    }
  }
  return EXIT_SUCCESS;
}
