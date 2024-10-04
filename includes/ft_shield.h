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


#define	sock_in		struct sockaddr_in
#define	sock_addr	struct sockaddr
#define	poll_fd		struct pollfd
#define LOCK_FILE "/var/lock/matt_daemon.lock"
// #define LOCK_FILE "/Users/cx02938/Desktop/matt_daemon.lock"
#define LOG_PATH  "/var/log/"
#define LOG_NAME  "ft_shield.log"
#define LOG_FILE  LOG_PATH LOG_NAME
// TODO meter dependencias hpp en Makefile
#define MAX_CLIENTS 3
#define MSG_SIZE	512


#endif