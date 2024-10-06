// include here all functions related to socket networking
#include "../includes/ft_shield.h"

void init_socket_struct(t_daemon *daemon) {
    // Init struct that the socket needs
	//  IPV4 addresses
	daemon->_addr.sin_family      = AF_INET;
	//  Convert our port to a network address (host to network)
	daemon->_addr.sin_port        = htons(DEFAULT_PORT);
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
		if (fd_ready(daemon) == 1)
			return ;
	}
}

bool    fd_ready( t_daemon *daemon )
{
	for (size_t i = 0; i < daemon->_pollfds_size; i++)
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
	daemon->_poll_fds[0] = (struct pollfd){fd, POLLIN, 0};
	daemon->_pollfds_size++;
}

void	delete_user(int pollfd_position, t_daemon *daemon)
{
	close(daemon->_poll_fds[pollfd_position].fd);
	daemon->_poll_fds[pollfd_position] = (struct pollfd){0, POLLIN, 0};
	daemon->_pollfds_size--;
}