#include "../includes/ft_shield.h"

int main ( void )
{
    t_daemon *daemon = create_daemon();
    server_listen(daemon);
    free(daemon);
    return (EXIT_SUCCESS);
}