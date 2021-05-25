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

int read_obj(void **obj, size_t *obj_size)
{
	int payload_fd;

	if ((payload_fd = open("payload/payload", O_RDONLY)) < 0)
	{
		printf("Error %d opening 'payload' file.\n", payload_fd);
		return (-1);
	}
	if ((*obj_size = lseek(payload_fd, (size_t)0, SEEK_END)) <= 0)
	{
		printf("Error empty payload\n");
		close(payload_fd);
		return (-1);
	}
	if ((*obj = mmap(0, *obj_size, PROT_READ, MAP_PRIVATE, payload_fd, 0)) == \
			MAP_FAILED)
	{
		printf("Error mapping payload\n");
		close(payload_fd);
		return (-1);
	}
	close(payload_fd);
	return 0;
}

int durex()
{
	// creates a Durex file
	int durex_fd;

	if ((durex_fd = open("_Durex", O_WRONLY | O_CREAT | O_TRUNC, 0755)) < 0)
	{
		printf("Error %d creating 'Durex' file.\n", durex_fd);
		return (-1);
	}
	// read payload content
	void *obj = NULL;
	size_t obj_size = 0;

	if (read_obj(&obj, &obj_size))
	{
		close(durex_fd);
		return (-1);
	}
	// write content
	write(durex_fd, obj, obj_size);
	// tmp
	if (munmap(obj, obj_size) < 0)
		printf("Error munmap\n");
	close(durex_fd);
	return 0;
}

int main(void)
{
	durex();
	printf("eparisot\n");
	return 0;
}