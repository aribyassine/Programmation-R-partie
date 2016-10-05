#include <unistd.h>
#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>

int nfork(int nb_fils){
	pid_t pid;
	int i = 0;
	while(i<nb_fils){
	pid = fork();
		switch (pid) {
		case -1:
			return -1;
		break;
		/* Si on est dans le fils */
		case 0:
			return 0;
		break;
		/* Si on est dans le père */
		default:
			i++;
		break;
		}
	}
	return i;
}

int main (void) {
  int p;
  int i=1; int N = 3;
  do {
    p = nfork (i) ;
    if (p != 0 )
      printf ("nb fils %d , pid %d\n",p,getpid());
	} while ((p ==0) && (++i<=N));
	printf ("FIN %d\n", getpid());
	return EXIT_SUCCESS;
}
