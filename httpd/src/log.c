#include "log.h"
#include <stdio.h>
#include <stdlib.h>

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
    printf("%s ", logs.ip);
    printf("%s ", logs.date);
    printf("%d ", logs.serveur_pid);
    printf("%d ", logs.thread_id);
    printf("%s ", logs.first_line);
    printf("%d ", logs.http_code);
    printf("%d \n", logs.file_size);
}
