#include "chat_common.h"

int main(int argc, char const *argv[]) {
    if (argc != 2) {
        printf("Nombre d'argument incorrect\n");
        return EXIT_FAILURE;
    }
    char const *client_name = argv[1];
    char const *serveur_name = argv[2];
    int fd_client;

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
    return EXIT_SUCCESS;
}
