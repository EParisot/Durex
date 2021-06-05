/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   payload.h                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: eparisot <eparisot@42.student.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2021/05/25 14:55:42 by eparisot          #+#    #+#             */
/*   Updated: 2021/05/25 14:55:42 by eparisot         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef PAYLOAD_H

#define PAYLOAD_H

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <string.h>
#include <errno.h>
#include <sys/time.h> //FD_SET, FD_ISSET, FD_ZERO macros 
#include <sys/wait.h>
#include <sys/mman.h>
#include <pthread.h>

#define DEBUG 1

#define REMOTE_ADDR "0.0.0.0"
#define REMOTE_PORT 4242
#define MAX_CLIENTS 3

#define KEY "secret__password"

typedef struct 	wait_args_s
{
	int 		pid;
	int 		*clients_sockets;
}				wait_args_t;

int 	rabbit(char *input, const char *key);

#endif