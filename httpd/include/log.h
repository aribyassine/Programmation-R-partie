typedef struct log_struct log_struct;
struct log_struct {
    int client_sock;
    char ip[15];
    char date[50];
    int serveur_pid;
    int thread_id;
    char first_line[1024];
    int http_code;
    int file_size;
};

void logging(log_struct);
#define LOGFILE "/tmp/http3525057.log"

