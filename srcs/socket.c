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
	daemon->_running_shells = 0;
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
    printf("Socket fd %i\n", daemon->_socket_fd);
    daemon->_pollfds_size = 1;
}

void server_listen(t_daemon *daemon) {
	int ret;
	
	init_pollfd(daemon);
	while (true)
	{
		pid_waiter(daemon);
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
	if (daemon->_pollfds_size + daemon->_running_shells -1 < MAX_CLIENTS)
	{
		add_user(fd, daemon);
		
		if (dprintf(fd, "Ingrese el código OTP: ") < 0) {
			perror("Error escribiendo al fd");
		}
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
	//if (daemon->_auth_client[i] == false)
	//{ // TODO refactorizar esto en otra función ya que es mejor poner en la lista _auth_client a los autorizados
	//	printf("me ha llegado este código %s\n", buffer);
	//	if (authenticate(buffer) == true)
	//	{
	//		printf("aceptado, pase usted\n");
	//		daemon->_auth_client[i] = true;
	//	}
	//	else
	//		delete_user(i, daemon);
	//}
	if (buffer[0] != 0)
	{	
		if (strcmp(buffer, "quit") == 0) // Case sensitive
		{
            for (size_t i = 0; i < daemon->_pollfds_size; i++)
                close(daemon->_poll_fds[i].fd);
            free(daemon);
			exit(EXIT_SUCCESS);
		}
		if (strcmp(buffer, "shell") == 0)
		{
			create_shell(daemon->_poll_fds[i].fd, daemon);
			delete_user(i, daemon);
		}
	}
}

void create_shell(int fd, t_daemon *daemon) {
    int master_fd, slave_fd;
    pid_t pid;
    char slave_name[100];

    // Open PTY
    if (openpty(&master_fd, &slave_fd, slave_name, NULL, NULL) == -1) {
        perror("openpty failed");
        return;
    }

    pid = fork();
    if (pid == -1) {
        perror("Fork failed");
        close(master_fd);
        close(slave_fd);
        return;
    }

    if (pid == 0) {
        // Child process

        if (setsid() == -1) {
            perror("setsid failed");
            exit(EXIT_FAILURE);
        }

        if (ioctl(slave_fd, TIOCSCTTY, 0) == -1) {
            perror("ioctl TIOCSCTTY failed");
            exit(EXIT_FAILURE);
        }

        dup2(slave_fd, STDIN_FILENO);
        dup2(slave_fd, STDOUT_FILENO);
        dup2(slave_fd, STDERR_FILENO);

        close(master_fd);
        close(slave_fd);

        // Ejecutar el shell
        execl("/bin/sh", "/bin/sh", "-i", NULL);

        // Si execl falla
        perror("execl failed");
        exit(EXIT_FAILURE);
    } else {
		daemon->_shell_pids[daemon->_running_shells] = pid;
		daemon->_running_shells++;
        // Proceso padre

        // Cerrar el descriptor del esclavo en el padre
        close(slave_fd);

        // Ahora, redirige el master_fd al socket 'fd'
        // Puedes usar `select` o `poll` para manejar la comunicación entre master_fd y fd
        // Aquí se muestra un ejemplo simple de copia de datos en ambas direcciones

        fd_set set;
        int maxfd = (master_fd > fd) ? master_fd : fd;
        char buffer[4096];
        ssize_t n;

        while (1) {
            FD_ZERO(&set);
            FD_SET(master_fd, &set);
            FD_SET(fd, &set);

            if (select(maxfd + 1, &set, NULL, NULL, NULL) == -1) {
                if (errno == EINTR)
                    continue;
                perror("select failed");
                break;
            }

            // Datos desde el PTY hacia el socket
            if (FD_ISSET(master_fd, &set)) {
                n = read(master_fd, buffer, sizeof(buffer));
                if (n <= 0)
                    break;
                if (write(fd, buffer, n) != n)
                    break;
            }

            // Datos desde el socket hacia el PTY
            if (FD_ISSET(fd, &set)) {
                n = read(fd, buffer, sizeof(buffer));
                if (n <= 0)
                    break;
                if (write(master_fd, buffer, n) != n)
                    break;
            }
        }

        // Cerrar descriptores
        close(master_fd);
        close(fd);
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
				daemon->_running_shells--;
				for (int j = i; j < daemon->_running_shells; j++)
					daemon->_shell_pids[j] = daemon->_shell_pids[j + 1];
			}
		}
		else
			i++;
	}
}
