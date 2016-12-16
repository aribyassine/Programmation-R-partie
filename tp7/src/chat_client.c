#include "chat_common.h"

char const *client_name;
char const *serveur_name;
struct message_client *my_msg;
struct myshm *msg;

void init() {
    int fd_client, fd_serveur;
    /* Créé le segment en lecture écriture */
    if ((fd_client = shm_open(client_name, O_RDWR | O_CREAT, 0666)) == -1) {
        perror("shm_open client_name");
        exit(EXIT_FAILURE);
    }
    /* Allouer au segment une taille*/
    if (ftruncate(fd_client, sizeof(struct myshm)) == -1) {
        perror("ftruncate shm");
        exit(EXIT_FAILURE);
    }
    if ((my_msg = mmap(NULL, sizeof(struct message_client), PROT_READ | PROT_WRITE, MAP_SHARED, fd_client, 0)) == MAP_FAILED) {
        perror("mmap");
        exit(EXIT_FAILURE);
    }
    /* Ouvrir le segment en lecture écriture */
    if ((fd_serveur = shm_open(serveur_name, O_RDWR, 0666)) == -1) {
        perror("shm_open Server_name");
        exit(EXIT_FAILURE);
    }
    if ((msg = mmap(NULL, sizeof(struct myshm), PROT_READ | PROT_WRITE, MAP_SHARED, fd_serveur, 0)) == MAP_FAILED) {
        perror("mmap");
        exit(EXIT_FAILURE);
    }
    if (sem_init(&(my_msg->mutex), 1, 1) == -1) {
        perror("mutex_init");
        exit(EXIT_FAILURE);
    }
    if (sem_init(&(my_msg->sem), 1, 0) == -1) {
        perror("sem_init");
        exit(EXIT_FAILURE);
    }
}
void *func_rec() {
    while (1) {
        sem_wait(&(my_msg->sem));
        sem_wait(&(my_msg->mutex));
        printf(" message recu : %s\n", my_msg->content);
        sem_post(&(my_msg->mutex));
    }
}
void *func_em() {
    sem_wait(&(msg->sem));
    msg->messages[msg->write].type = 1;
    strcpy(msg->messages[msg->write].content, client_name);
    msg->write++;
    msg->nb++;
    sem_post(&(msg->sem));
    char m[TAILLE_MESS];
    while (1) {
        printf("> ");
        /* Get the message, with size limit. */
        fgets(m, TAILLE_MESS, stdin);
        /* Remove trailing newline, if there. */
        if ((strlen(m) > 0) && (m[strlen(m) - 1] == '\n'))
            m[strlen(m) - 1] = '\0';
        sem_wait(&(msg->sem));
        strcpy(msg->messages[msg->write].content, m);
        msg->messages[msg->write].type = 2;
        msg->write++;
        msg->nb++;
        sem_post(&(msg->sem));
    }
}

void free_m() {
    munmap(my_msg, 10);
    shm_unlink(client_name);
}

void sig_hand(int sig) {
    struct message *m = malloc(sizeof(struct message));
    m->type = 3;
    strcpy(m->content, client_name);
    sem_wait(&(msg->sem));
    msg->messages[msg->write] = *m;
    msg->write++;
    msg->nb++;
    sem_post(&(msg->sem));
    free_m();
    printf("\nFin Client %s\n", client_name);
    free(m);
    exit(0);
}

int main(int argc, char const *argv[]) {
    if (argc != 3) {
        printf("Nombre d'argument incorrect\n");
        return EXIT_FAILURE;
    }
    client_name = argv[1];
    serveur_name = argv[2];
    init();
    sigset_t sig_proc;
    struct sigaction action;
    sigemptyset(&sig_proc);
    /* changer le traitement */
    action.sa_mask = sig_proc;
    action.sa_flags = 0;
    action.sa_handler = sig_hand;
    sigaction(SIGINT, &action, NULL);
    pthread_t t_em, t_rec;

    if (pthread_create(&t_em, NULL, func_em, NULL) != 0) {
        perror("pthread_create em");
        exit(0);
    }

    if (pthread_create(&t_rec, NULL, func_rec, NULL) != 0) {
        perror("pthread_create rec");
        exit(0);
    }
    pthread_join(t_em, NULL);
    pthread_join(t_rec, NULL);
    return EXIT_SUCCESS;
}
