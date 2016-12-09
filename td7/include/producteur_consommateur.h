#include <errno.h>
#include <fcntl.h> /* For O_* constants */
#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <sys/stat.h> /* For mode constants */
#include <unistd.h>
#define PRODUCTEUR                                                             \
  int c;                                                                       \
  while ((c = getchar()) != EOF) {                                             \
    push(c);                                                                   \
  }

#define CONSOMMATEUR                                                           \
  while (1) {                                                                  \
    putchar(pop());                                                            \
    fflush(stdout);                                                            \
  }
