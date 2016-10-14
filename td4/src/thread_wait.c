#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int total;
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t condition = PTHREAD_COND_INITIALIZER;

void *func_thread(void *arg) {
  static int nb_appel = 0;
  pthread_mutex_lock(&mutex); /* On verrouille le mutex */
  nb_appel++;
  int valeur = (int)(10 * ((double)rand()) / RAND_MAX);
  printf("tid: %d => valeur aléatoire: %d\n", nb_appel, valeur);
  total += valeur;
  pthread_mutex_unlock(&mutex); /* On déverrouille le mutex */

  if (nb_appel == 1) {
    /* On verrouille le mutex */
    pthread_mutex_lock(&mutex);
    /* On attend que la condition soit remplie */
    pthread_cond_wait(&condition, &mutex);
    /* On déverrouille le mutex */
    pthread_mutex_unlock(&mutex);
    printf("total valeurs aléatoires: %d\n", total);
  } else if (nb_appel == (int)arg) {
    /* On verrouille le mutex */
    pthread_mutex_lock(&mutex);
    /* On délivre le signal : condition remplie */
    pthread_cond_signal(&condition);
    /* On déverrouille le mutex */
    pthread_mutex_unlock(&mutex);
  }
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
