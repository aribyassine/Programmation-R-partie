#define _XOPEN_SOURCE 700
#define _REENTRANT
#include <errno.h>
#include <fcntl.h>
#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

sem_t *semaphore;
sem_t *barrier;

void wait_barrier(int NB_PCS) {
    if (!sem_trywait(semaphore))
        sem_wait(barrier);
    else {
        int i;
        for (i = 0; i < NB_PCS - 1; ++i) {
            sem_post(barrier);
            sem_post(semaphore);
        }
    }
}

/* La fonction create_process duplique le processus appelant et retourne
         le PID du processus fils ainsi créé */
pid_t create_process() {
    pid_t pid;
    do {
        pid = fork();
    } while ((pid == -1) && (errno == EAGAIN));
    return pid;
}

/* La fonction child_process effectue les actions du processus fils */
void child_process(int NB_PCS) {
    printf("avant barrière\n");
    wait_barrier(NB_PCS);
    printf("après barrière\n");
    exit(0);
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        printf("Nombre d'argument incorrect\n");
        return EXIT_FAILURE;
    }

    int nb_proc = atoi(argv[1]);
    /*creation d’un semaphore initialisé à 0 */
    if ((barrier = sem_open("/barrier", O_CREAT | O_EXCL | O_RDWR, 0666, 0)) == SEM_FAILED) {
        if (errno != EEXIST) {
            perror("sem_open");
            exit(1);
        }
    }
    /*creation d’un semaphore initialisé à n-1 */
    if ((semaphore = sem_open("/monsem", O_CREAT | O_EXCL | O_RDWR, 0666, nb_proc - 1)) == SEM_FAILED) {
        if (errno != EEXIST) {
            perror("sem_open");
            exit(1);
        }
    }
    pid_t pid;
    int n = 0;
    do {
        pid = create_process();
        switch (pid) {
        case -1:
            perror("fork");
            return EXIT_FAILURE;
            break;
        /* Si on est dans le fils */
        case 0:
            child_process(nb_proc);
            break;
        }
    } while ((pid != 0) && (++n < nb_proc));

    /* Fermer le semaphore */
    sem_close(semaphore);
    /* Detruire le semaphore */
    sem_unlink("/monsem");
    /* Fermer le semaphore */
    sem_close(barrier);
    /* Detruire le semaphore */
    sem_unlink("/barrier");

    return EXIT_SUCCESS;
}
