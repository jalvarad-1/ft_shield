#include "../includes/ft_shield.h"


typedef struct s_daemon
{
    sock_in         _addr;
    int             _lock_file_fd;
    int             _socket_fd;
    struct pollfd   _poll_fds[MAX_CLIENTS + 1];
    size_t          _pollfds_size;
} t_daemon;

t_daemon *create_daemon( void ) {
	//logger.log_entry("Creating server", "INFO");
	if (init_server() == 0) {
        perror("Server not listening");
        exit(EXIT_FAILURE);
	}
	//logger.log_entry("Server created", "INFO");
}
void init_socket_struct(t_daemon *daemon) {
    // Init struct that the socket needs
	//  IPV4 addresses
	daemon->_addr.sin_family      = AF_INET;
	//  Convert our port to a network address (host to network)
	daemon->_addr.sin_port        = htons(4242);
	//  Our address as integer
	daemon->_addr.sin_addr.s_addr = INADDR_ANY; // LOCALHOST
}

//pass with reserve of memory 
bool init_server(t_daemon *daemon) {
	int opt = 1;
	init_socket_struct(daemon);
	// Create the socket
	if ((daemon->_socket_fd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
		//logger.log_entry("Failed to create socket", "ERROR");
        perror("Failed to create socket");
		return(false);
	}
	// Set socket options (reuse address)
	if (setsockopt(daemon->_socket_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) == -1) {
		close(daemon->_socket_fd);
		//logger.log_entry("Failed to set socket options", "ERROR");
        perror("Failed to set socket options");
		return(false);
	}
	// Bind the socket to the address and port
	if (bind(daemon->_socket_fd, (const sock_addr*)&daemon->_addr, sizeof(daemon->_addr)) == -1) {
		close(daemon->_socket_fd);
			//logger.log_entry("Error binding socket", "ERROR");
        perror("Error binding socket");
		return(false);
	}
	// Start listening for connections, with MAX_CLIENTS backlog
	if (listen(daemon->_socket_fd, MAX_CLIENTS) == -1) {
		close(daemon->_socket_fd);
		//logger.log_entry("Error starting to listen on socket", "ERROR");
        perror("Error starting to listen on socket");
		return(false);
	}
	return(true);  // Successfully listening
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

void	init_pollfd(t_daemon *daemon)
{
	memset(daemon->_poll_fds, 0, sizeof(daemon->_poll_fds));
	daemon->_poll_fds[0] = (struct pollfd){daemon->_socket_fd, POLLIN, 0};
}

void server_listen(t_daemon *daemon) {
	int ret;
	
	init_pollfd(daemon);
	while (true)
	{
		ret = poll(daemon->_poll_fds, daemon->_pollfds_size, -1);
		if (ret < 0) {
			perror("Poll error");
			return;
		}
		if (ret == 0)
			continue;
		if (fd_ready() == 1)
			return ;
	}
}

bool    fd_ready( t_daemon *daemon )
{
	for (int i = 0; i < daemon->_pollfds_size; i++)
	{
		if (daemon->_poll_fds[i].revents == 0)
			continue;
		if (daemon->_poll_fds[i].fd == daemon->_socket_fd)
		{ 
			accept_communication(daemon);
			return 0;
		}
		else
		{
			receive_communication(i, daemon);
			return 0;
		}
	}
	return 1;
}

void	accept_communication(t_daemon *daemon)
{
	int 	fd = 0;
	sock_in client_addr;
	socklen_t client_addr_size = sizeof(client_addr);
	fd = accept(daemon->_socket_fd, (sock_addr*)&client_addr, &client_addr_size);
	if (fd < 0)
	{
		if (errno != EWOULDBLOCK)
			perror("  accept() failed");
		return ;
	}
	if (fcntl(fd, F_SETFL, O_NONBLOCK) == -1)
	{
		perror(" FCNTL failed");
		return ;
	}
	if (daemon->_pollfds_size -1 < MAX_CLIENTS)
	{
		add_user(fd, daemon);
		//logger.log_entry("New user connected", "INFO");
	}
	else
	{
		//logger.log_entry("Max clients reached. Connection refused.", "ERROR");
		close(fd);
	}
}

void	receive_communication(int i, t_daemon *daemon)
{
	char buffer[MSG_SIZE];
	int len;

	memset(buffer, 0, MSG_SIZE);
	len = recv(daemon->_poll_fds[i].fd, buffer, sizeof(buffer), 0);
	if (len < 0)
	{
		if (errno != EWOULDBLOCK)
			perror("  recv() failed");
		exit(EXIT_FAILURE);
	}
	if (len == 0)
	{
		delete_user(i, daemon);
		//logger.log_entry("Usuario desconectado", "INFO");
		return ;
	}
	buffer[len-1] = 0;
	if (buffer[0] != 0)
	{	
		// check if QUIT msg has been sent
		if (strcmp(buffer, "quit") == 0) // Character sensitive
		{
			//logger.log_entry("Request quit.", "INFO");
			//logger.log_entry("Quitting", "INFO");
			exit(EXIT_SUCCESS);
		}
		// add here a log entry with the message
		//std::string user_input("User input: ");
		//user_input+= buffer;
		//logger.log_entry(user_input, "LOG");
	}
}

void	add_user(int fd, t_daemon *daemon)
{
	poll_fd local_struct = {fd, POLLIN, 0};
	daemon->_poll_fds.push_back(local_struct);
}