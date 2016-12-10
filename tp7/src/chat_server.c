#include "chat_common.h"

struct myshm *msg;
char *clients_seg[MAX_USERS];
char *clients_ids[MAX_USERS];
int cpt_client = 0;

void connexion(char *id_client) {
    printf("> connexion : %s\n", id_client);
    clients_ids[cpt_client] = id_client;
    int fd;
    if ((fd = shm_open(id_client, O_RDWR, 0600)) == -1) {
        perror("shm_open connexion");
        exit(1);
    }
    if ((clients_seg[cpt_client] = mmap(NULL, sizeof(char) * TAILLE_MESS, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0) == MAP_FAILED)) {
        perror("mmap");
        exit(EXIT_FAILURE);
    }
    cpt_client++;
}

void diffusion(char *client_msg) {
    printf("> diffusion : %s\n", client_msg);
    int i;
    for (i = 0; i < cpt_client; i++) {
        strcpy(clients_seg[i], client_msg);
    }
}
void deconnexion(char *id_client) {
    printf("> deconnexion : %s\n", id_client);
    int i, y, n = cpt_client;
    for (i = 0; i < n; i++) {
        if (strcmp(clients_ids[i], id_client) == 0) {
            for (y = i; y < n; y++) {
                clients_ids[y] = clients_ids[y + 1];
                clients_seg[y] = clients_seg[y + 1];
                cpt_client--;
            }
            break;
        }
    }
}
int main(int argc, char const *argv[]) {
    if (argc != 2) {
        printf("Nombre d'argument incorrect\n");
        return EXIT_FAILURE;
    }
    char const *serveur_name = argv[1];
    int fd;
    /* Créé le segment en lecture écriture */
    if ((fd = shm_open(serveur_name, O_RDWR | O_CREAT, 0666)) == -1) {
        perror("shm_open serveur");
        exit(EXIT_FAILURE);
    }

    /* Allouer au segment une taille*/
    if (ftruncate(fd, sizeof(struct myshm)) == -1) {
        perror("ftruncate shm");
        exit(EXIT_FAILURE);
    }

    if ((msg = mmap(NULL, sizeof(struct myshm), PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0) == MAP_FAILED)) {
        perror("mmap");
        exit(EXIT_FAILURE);
    }

    /* init msg struct */
    msg->read = 0;
    msg->write = 0;
    msg->nb = 0;
    if (sem_init(&(msg->sem), 1, 1) == -1) {
        perror("sem_init");
        return EXIT_FAILURE;
    }

    while (1) {
        sem_wait(&(msg->sem));
        if (msg->write > 0) {
            struct message m = msg->messages[msg->write];
            msg->read++;
            msg->write--;
            switch (m.type) {
            case 1:
                connexion(m.content);
                break;
            case 2:
                diffusion(m.content);
                break;
            case 3:
                deconnexion(m.content);
                break;
            };
        }
        sem_post(&(msg->sem));
    }
    return EXIT_SUCCESS;
}
