/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   payload_builder.c                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: eparisot <eparisot@42.student.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2021/02/26 14:55:42 by eparisot          #+#    #+#             */
/*   Updated: 2021/02/26 14:55:42 by eparisot         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../includes/woody_woodpacker.h"

int 		get_payload(t_env *env, void *obj)
{
	// get obj header
	Elf64_Ehdr *ehdr = (Elf64_Ehdr *)obj;
	// get sections number
	int shnum = ehdr->e_shnum;
	// get first section header
	Elf64_Shdr *shdr = (Elf64_Shdr *)(obj + ehdr->e_shoff);
	// get str table
	Elf64_Shdr *sh_strtab = &shdr[ehdr->e_shstrndx];
	const char *sh_strtab_p = obj + sh_strtab->sh_offset;

	unsigned int start_offset = 0;

	for (int i = 0; i < shnum; ++i)
	{
		// get .text section
		if (ft_strequ(sh_strtab_p + shdr[i].sh_name, ".text"))
		{
			env->payload_size = shdr[i].sh_size;
			start_offset = shdr[i].sh_offset;		
		}
		if (ft_strequ(sh_strtab_p + shdr[i].sh_name, ".rodata"))
		{
			env->payload_size += shdr[i].sh_size;
			env->payload_rodata_size = shdr[i].sh_size;
		}
  	}

	if ((env->payload_content = malloc(env->payload_size)) == NULL)
		return 1;
	ft_bzero(env->payload_content, env->payload_size);
	ft_memcpy(env->payload_content, obj + start_offset, env->payload_size);
	//printf("DEBUG payload:");
	//debug_dump(env, env->payload_content, ehdr->e_entry, env->payload_size);
	return 0;
}

static int		read_obj(t_env *env, char *obj_name)
{
	int			fd;
	void		*obj;
	size_t		size;
	int ret = 	0;
	
	if ((fd = open(obj_name, O_RDONLY)) < 0)
	{
		print_err("Error openning file", obj_name);
		return 1;
	}
	if ((size = lseek(fd, (size_t)0, SEEK_END)) <= 0)
	{
		print_err("Error empty file", obj_name);
		close(fd);
		return 1;
	}
	if ((obj = mmap(0, size, PROT_READ, MAP_PRIVATE, fd, 0)) == \
			MAP_FAILED)
	{
		print_err("Error mapping file", obj_name);
		close(fd);
	}
	else
	{
		close(fd);
		// file mounted
		if (get_payload(env, obj))
			ret = 1;
		if (munmap(obj, size) < 0)
			print_err("Error munmap", "");
	}
	return ret;
}

int build_payload(t_env *env)
{
	int ret = 0;
	
	ret = read_obj(env, PAYLOAD_SRC);
	return ret; 
}
