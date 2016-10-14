#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int total;
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

void *func_thread(void *arg) {
  int valeur = (int)(10 * ((double)rand()) / RAND_MAX);
  printf("valeur aléatoire: %d\n", valeur);
  pthread_mutex_lock(&mutex); /* On verrouille le mutex */
  total += valeur;
  pthread_mutex_unlock(&mutex); /* On déverrouille le mutex */
  pthread_exit(NULL);
}

int main(int argc, char *argv[]) {
  if (argc != 2) {
    printf("Nombre d'argument incorrect\n");
    return EXIT_FAILURE;
  }
  total = 0;
  int nb_threads = atoi(argv[1]);
  int i, status[nb_threads];
  pthread_t tid[nb_threads];
  for (i = 0; i < nb_threads; i++) {
    if (pthread_create(&(tid[i]), NULL, func_thread, (void *)NULL) != 0) {
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
  printf("total valeurs aléatoires: %d\n", total);
  return EXIT_SUCCESS;
}
