#define _XOPEN_SOURCE 700
#define _REENTRANT
#include <errno.h>
#include <fcntl.h> /* For O_* constants */
#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <sys/stat.h> /* For mode constants */
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

sem_t *shm;

/* La fonction create_process duplique le processus appelant et retourne
         le PID du processus fils ainsi créé */
pid_t pid;
pid_t create_process() {
    do {
        pid = fork();
    } while ((pid == -1) && (errno == EAGAIN));
    return pid;
}

create_n_process(int nb_proc) {
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
            sleep(nb_proc % (n + 1));
            if (n == nb_proc - 1) {
                int i;
                for (i = 0; i < nb_proc - 1; i++) {
                    int val;
                    sem_getvalue(&shm[i], &val);
                    while (val == 1) {
                        sem_getvalue(&shm[i], &val);
                    }
                    sem_post(&shm[i]);
                    sleep(0.1);
                }
                printf("%d\n", n + 1);
            } else {
                sem_wait(&shm[n]);
                sem_wait(&shm[n]);
                printf("%d\n", n + 1);
            }
            break;
        default:
            break;
        }
    } while ((pid != 0) && (++n < nb_proc));
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        printf("Nombre d'argument incorrect\n");
        return EXIT_FAILURE;
    }
    int nb_proc = atoi(argv[1]);
    pid_t pidp = getpid();
    int shm_id;
    /* Créé le segment en lecture écriture */
    if ((shm_id = shm_open("/shm", O_RDWR | O_CREAT, 0666)) == -1) {
        perror("shm_open ");
        exit(EXIT_FAILURE);
    }

    /* Allouer au segment une taille*/
    if (ftruncate(shm_id, sizeof(int) * nb_proc) == -1) {
        perror("ftruncate shm");
        exit(EXIT_FAILURE);
    }
    if ((shm = mmap(NULL, sizeof(int) * nb_proc, PROT_READ | PROT_WRITE, MAP_SHARED, shm_id, 0)) == MAP_FAILED) {
        perror("mmap");
        exit(EXIT_FAILURE);
    }
    int i;
    for (i = 0; i < nb_proc; i++) {
        if (sem_init(&shm[i], 1, 1) == -1) {
            perror("sem_init ");
            exit(EXIT_FAILURE);
        }
    }
    create_n_process(nb_proc);

    /* Wait for children to exit. */
    int status;
    pid_t pid;
    int n = nb_proc;
    while (n > 0) {
        pid = wait(&status);
        /*printf("Child with PID %ld exited with status 0x%x.\n", (long)pid,
         * status);*/
        --n;
    }
    if (pidp == getpid()) {
        printf("fin\n");
    }
    /* “detacher” le segment */
    munmap(shm, sizeof(int));
    /* detruire le segment */
    shm_unlink("/shm");
    return EXIT_SUCCESS;
}
