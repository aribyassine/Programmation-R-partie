#define _XOPEN_SOURCE 700
#define _REENTRANT
#include "producteur_consommateur.h"
#include <errno.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#define buf_size 100

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t condition_vide = PTHREAD_COND_INITIALIZER;
pthread_cond_t condition_plein = PTHREAD_COND_INITIALIZER;

int stack_size = 0;
char stack[buf_size];

push(char c) {
  // printf("thread %d : %d\n", (int)pthread_self(), stack_size);
  pthread_mutex_lock(&mutex);
  if (stack_size > buf_size) {
    pthread_cond_wait(&condition_plein, &mutex);
  }
  stack[stack_size] = c;
  stack_size++;
  pthread_cond_signal(&condition_vide);
  pthread_mutex_unlock(&mutex);
}

char pop() {
  // printf("thread %d : %d\n", (int)pthread_self(), stack_size);
  pthread_mutex_lock(&mutex);
  if (stack_size < 0) {
    pthread_cond_wait(&condition_vide, &mutex);
  }
  char c = stack[stack_size];
  stack_size--;
  pthread_cond_signal(&condition_plein);
  pthread_mutex_unlock(&mutex);
  return c;
}

void *func_thread_producteur(void *arg) { PRODUCTEUR }
void *func_thread_consommateur(void *arg) { CONSOMMATEUR }

int main(int argc, char *argv[]) {
  if (argc != 3) {
    printf("Nombre d'argument incorrect\n");
    return EXIT_FAILURE;
  }
  int nb_prod = atoi(argv[1]);
  int nb_cons = atoi(argv[2]);

  pthread_t tid_cons[nb_cons];
  pthread_t tid_prod[nb_prod];
  for (size_t i = 0; i < nb_prod; i++) {
    if (pthread_create(&(tid_prod[i]), NULL, func_thread_producteur, NULL) !=
        0) {
      perror("pthread_create");
      return EXIT_FAILURE;
    }
  }

  for (size_t i = 0; i < nb_cons; i++) {
    if (pthread_create(&(tid_cons[i]), NULL, func_thread_consommateur, NULL) !=
        0) {
      perror("pthread_create");
      return EXIT_FAILURE;
    }
  }
  for (size_t i = 0; i < nb_prod; i++) {
    if (pthread_join(tid_prod[i], NULL) != 0) {
      perror("error pthread_join");
      return EXIT_FAILURE;
    }
  }
  for (size_t i = 0; i < nb_cons; i++) {
    if (pthread_join(tid_cons[i], NULL) != 0) {
      perror("error pthread_join");
      return EXIT_FAILURE;
    }
  }

  return EXIT_SUCCESS;
}
