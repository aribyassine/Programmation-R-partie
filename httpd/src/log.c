#include "log.h"
#include <stdio.h>
#include <stdlib.h>
#define LOGFILE "tmp/http3525057.log"

void logging(char *msg) {
    FILE *log = fopen(LOGFILE, "a+");
    if (!log) {
        perror("fopen logfile");
        exit(EXIT_FAILURE);
    }
    fprintf(log, "%s", msg);
    fclose(log);
}
