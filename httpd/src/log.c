#include "log.h"
#include <stdio.h>
#include <stdlib.h>
#define LOGFILE "tmp/http3525057.log"

void logging(log_struct logs) {
    FILE *log = fopen(LOGFILE, "a+");
    if (!log) {
        perror("fopen logfile");
        exit(EXIT_FAILURE);
    }
    fprintf(log, "%s ", logs.ip);
    fprintf(log, "%s ", logs.date);
    fprintf(log, "%d ", logs.serveur_pid);
    fprintf(log, "%d ", logs.thread_id);
    fprintf(log, "%s ", logs.first_line);
    fprintf(log, "%d ", logs.http_code);
    fprintf(log, "%d \n", logs.file_size);
    fclose(log);
}
