/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   tools.c                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: eparisot <eparisot@42.student.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2021/02/26 14:55:42 by eparisot          #+#    #+#             */
/*   Updated: 2021/02/26 14:55:42 by eparisot         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../includes/woody_woodpacker.h"

void clear_env(t_env *env)
{
	if (env->obj_cpy)
		free(env->obj_cpy);
	if (env->payload_content)
		free(env->payload_content);
	free(env);
}

int dump_obj(t_env *env)
{
	int fd;

	if ((fd = open("woody", O_WRONLY | O_CREAT | O_TRUNC, 0755)) < 0)
	{
		printf("Error %d creating 'woody' file.\n", fd);
		return (-1);
	}
	write(fd, env->obj_cpy, (env->found_code_cave) ? env->obj_size : env->page_offset + env->payload_size);
	close(fd);
	return (0);
}

unsigned int replace_addr(t_env *env, unsigned int needle, unsigned int replace)
{
	size_t i = 0;
	int j = 0;

	for (i = 0; i < env->payload_size; ++i)
	{
		if (i * 8 < env->payload_size)
		{
			int found = 0;
			for (j = 0; j < 8; ++j)
			{
				if (i * 8 + j + 4 < env->payload_size && *(unsigned int *)(&((unsigned char *)(&((long unsigned int *)env->payload_content)[i]))[j]) == needle)
				{
					found = 1;
					break;
				}
			}
			if (found)
			{
				*(unsigned int *)(&((unsigned char *)(&((long unsigned int *)env->payload_content)[i]))[j]) = replace;
				break;

			}
		}
	}
	return (i * 8 + j);
}

int generate_key(t_env *env)
{
	int fd = 0;

	if ((fd = open("/dev/urandom", O_RDONLY)) < 0)
		return 1;
	if (read(fd, env->key, 16) < 0)
		return 1;
	env->key[16] = 0;
	return 0;
}

void print_key(t_env *env)
{
	printf("key_value: ");
	for (size_t i = 0; i < 16; ++i)
	{
		printf("%02x", env->key[i]);
	}
	printf("\n");
}

int print_err(char *err, char *arg)
{
	ft_putstr(err);
	if (ft_strlen(arg))
	{
		ft_putstr(" '");
		ft_putstr(arg);
		ft_putstr("'");
	}
	ft_putchar('\n');
	return (1);
}

uint32_t cpu_32(uint32_t n, uint8_t cpu)
{
	if (cpu != 0)
		return (ft_swap_32(n));
	return (n);
}

uint64_t cpu_64(uint64_t n, uint8_t cpu)
{
	if (cpu != 0)
		return (ft_swap_64(n));
	return (n);
}

void debug_dump(t_env *env, unsigned int *content, unsigned int start_addr, size_t size)
{
	printf("\nDEBUG: size = %ld bytes", size);
	for (size_t j = 0; j * 4 < size; j += 1)
	{
		if (j % 4 == 0)
			printf("\n %04lx - ", start_addr + j * 4);
		printf("%08x ", cpu_32(content[j], env->cpu));
	}
	printf("\n");
}

void debug_shdr(Elf64_Shdr shdr, char *label, const char *sh_strtab_p)
{
	printf("\nDEBUG: %s SHdr\n", label);
	printf("sh_name: %s\n", sh_strtab_p + shdr.sh_name);
	printf("sh_offset: %08lx\n", shdr.sh_offset);
	printf("sh_addr: %08lx\n", shdr.sh_addr);
	printf("sh_size: %08lx\n", shdr.sh_size);
	printf("sh_flags: %04lx\n", shdr.sh_flags);
	printf("sh_addralign: %02lx\n", shdr.sh_addralign);
}

void debug_phdr(Elf64_Phdr phdr, char *label)
{
	printf("\nDEBUG: %s PHdr\n", label);
	printf("p_offset: %08lx\n", phdr.p_offset);
	printf("p_vaddr: %08lx\n", phdr.p_vaddr);
	printf("p_paddr: %08lx\n", phdr.p_paddr);
	printf("p_filesz: %08lx\n", phdr.p_filesz);
	printf("p_memsz: %08lx\n", phdr.p_memsz);
	printf("p_flags: %04x\n", phdr.p_flags);
	printf("p_align: %02lx\n", phdr.p_align);
}

int check_corruption(void *obj, size_t size, char *obj_name)
{
	if (((char*)obj)[0] != 0x7f || \
		((char*)obj)[1] != 'E' || \
		((char*)obj)[2] != 'L' || \
		((char*)obj)[3] != 'F' || \
		((char*)obj)[4] != ELFCLASS64)
	{
		printf("%s is not an ELF64. Exiting...\n", obj_name);
		return -1;
	}
	// get obj header
	Elf64_Ehdr *ehdr = (Elf64_Ehdr *)obj;
	// get pheader number
	int phnum = ehdr->e_phnum;
	// get first p_header
	Elf64_Phdr *phdr = (Elf64_Phdr *)(obj + ehdr->e_phoff);
	if (ehdr->e_type > 3)
	{
		printf("Corrupted e_type %d in %s. Exiting...\n", ehdr->e_type, obj_name);
		return -1;
	}
	if (ehdr->e_ehsize >= 65535 || ehdr->e_ehsize <= 0)
	{
		printf("Corrupted ehsize %d in %s. Exiting...\n", ehdr->e_ehsize, obj_name);
		return -1;
	}
	if (ehdr->e_phoff != ehdr->e_ehsize)
	{
		printf("Corrupted e_phoff %ld in %s. Exiting...\n", ehdr->e_phoff, obj_name);
		return -1;
	}
	if (phnum >= 65535 || phnum <= 0)
	{
		printf("Corrupted phnum %d in %s. Exiting...\n", ehdr->e_phnum, obj_name);
		return -1;
	}
	if (ehdr->e_phentsize >= 65535 || ehdr->e_phentsize <= 0)
	{
		printf("Corrupted phentsize %d in %s. Exiting...\n", ehdr->e_phentsize, obj_name);
		return -1;
	}
	if (ehdr->e_shentsize >= 65535 || ehdr->e_shentsize <= 0)
	{
		printf("Corrupted shentsize %d in %s. Exiting...\n", ehdr->e_shentsize, obj_name);
		return -1;
	}
	if (ehdr->e_shstrndx >= 65535 || ehdr->e_shstrndx <= 0)
	{
		printf("Corrupted shstrndx %d in %s. Exiting...\n", ehdr->e_shstrndx, obj_name);
		return -1;
	}
	
	// get sections number
	int shnum = ehdr->e_shnum;
	// get first section header
	Elf64_Shdr *shdr = (Elf64_Shdr *)(obj + ehdr->e_shoff);

	if (shnum >= 65535 || shnum <= 0)
	{
		printf("Corrupted shnum %d in %s. Exiting...\n", ehdr->e_shnum, obj_name);
		return -1;
	}
	
	// get str table
	Elf64_Shdr *sh_strtab = &shdr[ehdr->e_shstrndx];
	if (sh_strtab->sh_offset >= size || sh_strtab->sh_offset <= 0)
	{
		printf("Corrupted shstrtab_offset %ld in %s. Exiting...\n", sh_strtab->sh_offset, obj_name);
		return -1;
	}
	const char *sh_strtab_p = obj + sh_strtab->sh_offset;
	int prev_type = 0;
	int load_found = 0;
	unsigned int obj_base = 0;
	for (int i = 0; i < phnum; ++i)
	{
		if (prev_type == PT_DYNAMIC && phdr[i].p_type == PT_LOAD)
		{
			printf("It is likely that %s have already been infected with PT_LOAD following a PT_NOTE. \nExiting...\n", obj_name);
			return -1;
		}
		if (phdr[i].p_type == 0)
		{
			printf("Corrupted p_type %d in %s. Exiting...\n", phdr[i].p_type, obj_name);
			return -1;
		}
		// get base address
		if (load_found == 0 && phdr[i].p_type == PT_LOAD)
		{
			obj_base = phdr[i].p_vaddr;
			load_found = 1;
		}
		if (phdr[i].p_type == PT_LOAD && (phdr[i].p_flags & 5) == 5 && i + 1 < phnum)
		{
			if (phdr[i].p_filesz >= 0xffffffffffffffff || (int)phdr[i].p_filesz <= 0)
			{
				printf("Corrupted p_filesz %ld in %s. Exiting...\n", phdr[i].p_filesz, obj_name);
				return -1;
			}
			if (phdr[i].p_memsz >= 0xffffffffffffffff || (int)phdr[i].p_memsz <= 0)
			{
				printf("Corrupted p_memsz %ld in %s. Exiting...\n", phdr[i].p_memsz, obj_name);
				return -1;
			}
			if (phdr[i].p_offset >= 0xffffffffffffffff)
			{
				printf("Corrupted p_offset %ld in %s. Exiting...\n", phdr[i].p_offset, obj_name);
				return -1;
			}
			if (phdr[i].p_paddr >= 0xffffffffffffffff)
			{
				printf("Corrupted p_paddr %ld in %s. Exiting...\n", phdr[i].p_paddr, obj_name);
				return -1;
			}
			if (phdr[i].p_vaddr >= 0xffffffffffffffff)
			{
				printf("Corrupted p_vaddr %ld in %s. Exiting...\n", phdr[i].p_vaddr, obj_name);
				return -1;
			}
		}
		prev_type = phdr[i].p_type;
	}

	for (int i = 0; i < shnum; ++i)
	{
		if ((int)shdr[i].sh_name < 0 || sh_strtab->sh_offset + shdr[i].sh_name > size)
		{
			printf("Corrupted sh_name %d in %s. Exiting...\n", shdr[i].sh_name, obj_name);
			return -1;
		}
		if (ft_strequ(sh_strtab_p + shdr[i].sh_name, ".fini") && ehdr->e_entry - obj_base > shdr[i].sh_offset)
		{
			printf("It is likely that %s have already been infected in PT_LOAD code cave. \nExiting...\n", obj_name);
			return -1;
		}
		if (ft_strequ(sh_strtab_p + shdr[i].sh_name, ".text"))
		{
			if (shdr[i].sh_addr >= 0xffffffffffffffff || shdr[i].sh_addr <= 0)
			{
				printf("Corrupted sh_addr %ld in %s. Exiting...\n", shdr[i].sh_addr, obj_name);
				return -1;
			}
			if (shdr[i].sh_offset >= 0xffffffffffffffff || shdr[i].sh_offset <= 0)
			{
				printf("Corrupted sh_offset %ld in %s. Exiting...\n", shdr[i].sh_offset, obj_name);
				return -1;
			}
			if (shdr[i].sh_size >= 0xffffffffffffffff || shdr[i].sh_size <= 0)
			{
				printf("Corrupted sh_size %ld in %s. Exiting...\n", shdr[i].sh_size, obj_name);
				return -1;
			}
		}
	}
	return 0;
}