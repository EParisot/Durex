/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   client.c                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: eparisot <eparisot@42.student.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2021/05/25 14:55:42 by eparisot          #+#    #+#             */
/*   Updated: 2021/05/25 14:55:42 by eparisot         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../includes/client.h"

int main(int argc, char *argv[])
{
	char key[1025];
	memset(key, 0, 1025);
	//parse args
	struct sockaddr_in server_addr;
	server_addr.sin_family = AF_INET;
	if(inet_pton(AF_INET, ADDR, &server_addr.sin_addr) <= 0) 
	{
		printf("error: Invalid address/ Address not supported \n");
		return -1;
	}
	int port = PORT;
	if (argc > 1)
	{
		for (int i = 0; i < argc; ++i)
		{
			if (i+1 < argc && strcmp(argv[i], "-p") == 0 && argv[i+1])
			{
				port = atoi(argv[i+1]);
				if (port <= 0 || port >= 65535)
				{
					port = PORT;
				}
			}
			if (i+1 < argc && strcmp(argv[i], "-s") == 0 && argv[i+1])
			{
				if(inet_pton(AF_INET, argv[i+1], &server_addr.sin_addr) <= 0) 
				{
					printf("error: Invalid address/ Address not supported \n");
					return -1;
				}
			}
			if (i+1 < argc && strcmp(argv[i], "-k") == 0 && argv[i+1])
			{
				if (strlen(argv[i+1]) == 16)
				{
					memcpy(key, argv[i+1], 16);
				}
				else
				{
					printf("error: invalid key len (16 needed)\n");
					return -1;
				}
			}
		}
	}
	server_addr.sin_port = htons(port);
	// setup socket
	int sock = 0;
	int valread;
	int secured = 0;
    char buffer[1025];

	memset(buffer, 0, 1025);
	while (1)
	{
		if (secured == 0)
		{
			if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0)
			{
				printf("error: Creating Socket\n");
				return -1;
			}
			// connect to server
			if (connect(sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
			{
				printf("error: Connection Failed \n");
				return -1;
			}
			valread = read(sock, buffer, 1024);
			buffer[16] = 0;
			// key exchange
			if (strlen(key) == 0)
			{
				printf("Password: ");
				memset(key, 0, 1025);
				struct termios tp, save;
				tcgetattr( STDIN_FILENO, &tp);              /* get existing terminal properties */
				save = tp;                                  /* save existing terminal properties */
				tp.c_lflag &= ~ECHO;                        /* only cause terminal echo off */
				tcsetattr(STDIN_FILENO, TCSAFLUSH, &tp);    /* set terminal settings */
				if(fgets(key, 1024, stdin) == NULL)
				{
					printf("error: reading stdin failed\n");
					break;
				}
				tcsetattr( STDIN_FILENO, TCSANOW, &save);   /* restore original terminal settings */
			}
			if (rabbit(buffer, key))
			{
				printf("error: encrypting key\n");
				break;
			}
			
			send(sock, buffer, 17, 0);
			valread = read(sock, buffer, 1024);
			buffer[valread] = 0;
			printf("%s", buffer);
			if (strncmp(buffer, "Welcome", 7) == 0)
			{
				secured = 1;
			}
			else
			{
				close(sock);
				sleep(1);
				continue;
			}
		}
		else
		{
			// communicate
			if(fgets(buffer, 1024, stdin) == NULL)
			{
				printf("error: reading stdin failed\n");
			}
			send(sock, buffer, strlen(buffer), 0);
			valread = read(sock , buffer, 1024);
			buffer[valread] = 0;
			printf("%s", buffer);
		}
	}
	return 0;
}