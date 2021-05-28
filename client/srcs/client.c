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
	//parse args
	struct sockaddr_in server_addr;
	server_addr.sin_family = AF_INET;
	int port = 0;
	if (argc > 1)
	{
		for (int i = 0; i < argc; ++i)
		{
			if (i+1 < argc && strcmp(argv[i], "-p") == 0)
			{
				port = atoi(argv[i+1]);
			}
			if (i+1 < argc && strcmp(argv[i], "-s") == 0)
			{
				if(inet_pton(AF_INET, argv[i+1], &server_addr.sin_addr) <= 0) 
				{
					printf("error: Invalid address/ Address not supported \n");
					return -1;
				}
			}
		}
		if (port <= 0 || port >= 65535)
		{
			port = PORT;
		}
	}
	else
	{
		port = PORT;
		if(inet_pton(AF_INET, ADDR, &server_addr.sin_addr) <= 0) 
		{
			printf("error: Invalid address/ Address not supported \n");
			return -1;
		}
	}
	server_addr.sin_port = htons(port);
	// setup socket
	int sock = 0;
	int valread;
	int secured = 0;
    
    char buffer[1025] = {0};

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
	while (1)
	{
		if (secured == 0)
		{
			valread = read(sock , buffer, 1024);
			char password[16];
			memset(password, 0, 16);
			if (valread >= 0 && strncmp(buffer, "Password: ", 10) == 0)
			{
				buffer[valread] = '\0';
				printf("%s", buffer);
				// key exchange
				if(fgets(password, 16, stdin) == NULL)
				{
					printf("error: reading stdin failed\n");
				}
				send(sock, password, strlen(password), 0);
				valread = read(sock , buffer, 1024);
				buffer[valread] = '\0';
				printf("%s", buffer);
				if (strncmp(buffer, "Welcome", 7) == 0)
				{
					secured = 1;
				}
				else
				{
					continue;
				}
			}
		}
		else
		{
			continue;
		}
		// communicate
	}
	return 0;
}