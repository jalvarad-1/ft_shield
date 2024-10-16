#include "../includes/ft_shield.h"

t_daemon *create_daemon( void ) {
	//logger.log_entry("Creating server", "INFO");
	t_daemon *daemon;
	//ft_daemonize();// TODO 	quitar esto
	daemon = malloc(sizeof(t_daemon));
	if (init_server(daemon) == 0) {
        perror("Server not listening");
        exit(EXIT_FAILURE);
	}
	//logger.log_entry("Server created", "INFO");
	return daemon;
}

// From the original Daemonize function
/* (This function forks, and if the fork(2) succeeds, the parent
			 calls _exit(2), so that further errors are seen by the child
			 only.)  On success Daemon() returns zero.  If an error occurs,
			 Daemon() returns -1 and sets errno to any of the errors specified
			 for the fork(2) and setsid(2). */

void ft_daemonize(void) {
	pid_t pid = fork(); 

	if (pid == -1) { 
		//logger.log_entry(strerror(errno), "ERROR");
		exit(EXIT_FAILURE); 
	} 
	if (pid > 0) { 
		exit(EXIT_SUCCESS);
	}
	//logger.log_entry("Entering Daemon mode", "INFO");
	//logger.log_entry("started. PID: " + std::to_string(getpid()) + ".", "INFO");
}

// https://stackoverflow.com/questions/1599459/optimal-lock-file-method
void create_lock_file(t_daemon *daemon) {
	// create file
	daemon->_lock_file_fd = open(LOCK_FILE, O_RDWR | O_CREAT, 0666);
	if (daemon->_lock_file_fd < 0) {
		perror( "Can't open or creating: "LOCK_FILE);
		//logger.log_entry("Error creating lock file", "ERROR");
		//logger.log_entry("Quitting", "INFO");
		exit(EXIT_FAILURE);
	}
	// lock file
	if (flock(daemon->_lock_file_fd, LOCK_EX | LOCK_NB) == -1)
	{
        perror( "Can't open: "LOCK_FILE);
		//logger.log_entry("Error file locked", "ERROR");
		//logger.log_entry("Quitting", "INFO");
		exit(EXIT_FAILURE);
	}
}