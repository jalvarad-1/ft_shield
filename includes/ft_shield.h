#pragma once
#ifndef FT_SHIELD_H
#define FT_SHIELD_H
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <syslog.h>
#include <errno.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <fcntl.h>
#include <sys/file.h>
#include <signal.h>
#include <poll.h>
#include <time.h>
#include <string.h>


#define	sock_in		struct sockaddr_in
#define	sock_addr	struct sockaddr
#define	poll_fd		struct pollfd
#define LOCK_FILE "/var/lock/matt_daemon.lock"
// #define LOCK_FILE "/Users/cx02938/Desktop/matt_daemon.lock"
#define LOG_PATH  "/var/log/"
#define LOG_NAME  "ft_shield.log"
#define LOG_FILE  LOG_PATH LOG_NAME
#define MAX_CLIENTS 3
#define MSG_SIZE	512
#define DEFAULT_PORT 4242

typedef struct s_daemon
{
    sock_in         _addr;
    int             _lock_file_fd;
    int             _socket_fd;
    struct pollfd   _poll_fds[MAX_CLIENTS + 1];
    size_t          _pollfds_size;
} t_daemon;

t_daemon    *create_daemon( void );
void        init_socket_struct(t_daemon *daemon);
bool        init_server(t_daemon *daemon);
void        ft_daemonize(void);
void        create_lock_file(t_daemon *daemon);
void        init_pollfd(t_daemon *daemon);
void        server_listen(t_daemon *daemon);
bool        fd_ready( t_daemon *daemon );
void        accept_communication( t_daemon *daemon);
void        receive_communication(int i, t_daemon *daemon);
void        add_user(int fd, t_daemon *daemon);
void        delete_user(int pollfd_position, t_daemon *daemon);

#endif