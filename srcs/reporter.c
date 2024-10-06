#include "../includes/ft_shield.h"

FILE * create_logfile() {
    return (fopen(LOG_FILE, "a"));
}

char* time_formatted() {
    const int MAXLEN = 80;
    char s[MAXLEN];
    time_t t = time(NULL);
    struct tm* local_time = localtime(&t);

    strftime(s, sizeof(s), "%d/%m/%Y-%H:%M:%S", local_time);

    char* result = malloc(strlen(s) + 1);
    if (result == NULL) {
        fprintf(stderr, "Error: No se pudo asignar memoria\n");
        return NULL;
    }
    strcpy(result, s);
    return result;
}
// TODO probar eliminar el archivo mientras se ejecuta
// TODO BONUS ocultar archivo de logs o encriptarlo 
bool log_entry(char * log_msg, char * log_level, FILE *logger) {
    if (logger == NULL)
        return false;
    char* log_time = time_formatted();
    if (log_time == NULL) {
        return false;
    }
    if (fprintf(logger, "[%s][ %s ] - Matt_daemon: %s\n", log_time, log_level, log_msg) < 0)
    {
        free(log_time);
        return false;
    }
    fflush(logger);
    free(log_time);
    return true;
}
