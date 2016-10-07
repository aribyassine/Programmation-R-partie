#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>

int main(void) {
  int p;
  int i = 1;
  int N = 3;
  do {
    p = fork();
    if (p != 0)
      printf("fils %d , pid %d\n", p, getpid());
  } while ((p == 0) && (++i <= N));
  printf("FIN %d\n", getpid());
  return EXIT_SUCCESS;
}
