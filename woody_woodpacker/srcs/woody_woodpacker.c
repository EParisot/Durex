/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   woody_woodpacker.c                                 :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: eparisot <eparisot@42.student.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2021/02/26 14:55:42 by eparisot          #+#    #+#             */
/*   Updated: 2021/02/26 14:55:42 by eparisot         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../includes/woody_woodpacker.h"

static void inject_code(t_env *env)
{
	// replace entrypoint
	((Elf64_Ehdr *)env->obj_cpy)->e_entry = env->inject_addr; // new entrybpoint + base addr

	// replace start addr in payload (this is a negative offset)
	replace_addr(env, 0x39393939, -(env->inject_dist + LD_OFFSET));
	// replace encrypt size in payload
	replace_addr(env, 0x40404040, (int)env->encrypt_size);
	// replace key addr in payload
	replace_addr(env, 0x41414141, (*(long unsigned int*)env->key << 32) >> 32);
	replace_addr(env, 0x41414141, *(long unsigned int*)env->key >> 32);
	replace_addr(env, 0x41414141, (*(long unsigned int*)(env->key+8) << 32) >> 32);
	replace_addr(env, 0x41414141, (*(long unsigned int*)(env->key+8)) >> 32);
	
	// replace jmp addr in payload
	replace_addr(env, 0x42424242, -env->inject_dist - env->payload_size + 0x17 + env->payload_rodata_size);

	// inject payload
	ft_memmove(env->obj_cpy + env->inject_offset, env->payload_content, env->payload_size);
}

static int parse_elf(t_env *env) 
{
	// get obj header
	Elf64_Ehdr *ehdr = (Elf64_Ehdr *)env->obj_cpy;
	// get sections number
	int shnum = ehdr->e_shnum;
	// get first section header
	Elf64_Shdr *shdr = (Elf64_Shdr *)(env->obj_cpy + ehdr->e_shoff);
	// get pheader number
	int phnum = ehdr->e_phnum;
	// get first p_header
	Elf64_Phdr *phdr = (Elf64_Phdr *)(env->obj_cpy + ehdr->e_phoff);
	// get str table
	Elf64_Shdr *sh_strtab = &shdr[ehdr->e_shstrndx];
	const char *sh_strtab_p = env->obj_cpy + sh_strtab->sh_offset;
	
	// get original entrypoint
	env->entrypoint = ehdr->e_entry;
	
	int load_found = 0;
	for (int i = 0; i < phnum; ++i)
	{
		// get base address
		if (load_found == 0 && phdr[i].p_type == PT_LOAD)
		{
			env->obj_base = phdr[i].p_vaddr;
			env->load_align = phdr[i].p_align;
			// set the text address
			env->text_addr = (char*)env->obj_cpy + (env->entrypoint - env->obj_base);
			load_found = 1;
		}
		// find code cave in executable segment
		if (phdr[i].p_type == PT_LOAD && (phdr[i].p_flags & 5) == 5 && i + 1 < phnum)
		{
			if (phdr[i+1].p_offset - (phdr[i].p_offset + phdr[i].p_memsz) > env->payload_size)
			{
				env->inject_offset = phdr[i].p_offset + phdr[i].p_memsz;
				env->inject_addr = env->inject_offset + env->obj_base;
				env->found_code_cave = 1;
				// patch the pheader
				phdr[i].p_filesz += env->payload_size;
				phdr[i].p_memsz += env->payload_size;
				phdr[i].p_flags = PF_R | PF_W | PF_X;
			}
			else 
			{
				printf("Not enought place in PT_LOAD, injecting at end of file using PT_NOTE...\n");
				// set the .text header rights too
				env->inject_offset = env->page_offset;
				env->inject_addr = env->inject_offset + env->obj_base;
				phdr[i].p_flags = PF_R | PF_W | PF_X;
			}
			unsigned int fini = 0;
			for (int j = 0; j < shnum; ++j)
			{
				if (ft_strequ(sh_strtab_p + shdr[j].sh_name, ".fini"))
				{
					fini = shdr[j].sh_offset;
				}
			}
			// set the size to encrypt from entrypoint
			env->encrypt_size = fini - (env->entrypoint - env->obj_base);
			// set dist between .text end and inject point
			env->inject_dist = env->inject_addr - env->entrypoint;
		}
		// get .note.* phdr if no code_cave found (so it will become the new injected phdr)
		if (env->found_code_cave == 0 && phdr[i].p_type == PT_NOTE)
		{
			// set the new injected phdr
			phdr[i].p_type = PT_LOAD;
			phdr[i].p_offset = env->inject_offset;
			phdr[i].p_paddr = env->inject_addr;
			phdr[i].p_vaddr = env->inject_addr;
			phdr[i].p_filesz = env->payload_size;
			phdr[i].p_memsz = env->payload_size;
			phdr[i].p_flags = PF_R | PF_W | PF_X;
			phdr[i].p_align = env->load_align;
			break;
		}
	}
	if (env->found_code_cave == 0)
	{
		for (int i = 0; i < shnum; ++i)
		{
			// get .note.ABI-tag shdr if no code_cave found (so it will become the new injected phdr)
			if (shdr[i].sh_type == SHT_NOTE && ft_strequ(sh_strtab_p + shdr[i].sh_name, ".note.ABI-tag"))
			{
				// set the new injected shdr
				shdr[i].sh_type = SHT_PROGBITS;
				shdr[i].sh_offset = env->inject_offset;
				shdr[i].sh_addr = env->inject_addr;
				shdr[i].sh_size = env->payload_size;
				shdr[i].sh_flags = SHF_ALLOC | SHF_EXECINSTR;
				shdr[i].sh_addralign = 16;
				continue;
			}
		}
	}
	return 0;
}

static void get_bss_size(t_env *env) 
{
	// get obj header
	Elf64_Ehdr *ehdr = (Elf64_Ehdr *)env->obj;
	// get sections number
	int shnum = ehdr->e_shnum;
	// get first section header
	Elf64_Shdr *shdr = (Elf64_Shdr *)(env->obj + ehdr->e_shoff);
	// get str table
	Elf64_Shdr *sh_strtab = &shdr[ehdr->e_shstrndx];
	const char *sh_strtab_p = env->obj + sh_strtab->sh_offset;

	for (int i = 0; i < shnum; ++i)
	{
		if (ft_strequ(sh_strtab_p + shdr[i].sh_name, ".bss"))
		{
			env->bss_offset = shdr[i].sh_addr;
			env->bss_size = shdr[i].sh_size;
			while (env->bss_size % shdr[i].sh_addralign)
			{
				++env->bss_size;
			}
		}
	}
}

static int handle_obj(t_env *env)
{
	build_payload(env);
	get_bss_size(env);
	// inject after .bss
	env->page_offset = env->obj_base + env->bss_offset + env->bss_size;
	if (env->page_offset + env->payload_size > env->obj_size)
	{
		env->new_obj_size = env->page_offset + env->payload_size;
	}
	// copy original file
	if ((env->obj_cpy = malloc(env->new_obj_size)) == NULL)
	{
		printf("Error: can't duplicate file.\n");
		return 1;
	}
	ft_bzero(env->obj_cpy, env->new_obj_size);
	ft_memcpy(env->obj_cpy, env->obj, env->obj_size);

	// get .text content
	if (parse_elf(env))
	{
		printf("Error parsing elf.\n");
		return 1;
	}

	printf("Original entrypoint: \t%08x\n", env->entrypoint);
	printf("Inserted entrypoint: \t%08x\n", env->inject_addr);
	printf("Inserted size: \t\t%08lx\n", env->payload_size);

	// generate Key
	if (generate_key(env))
	{
		printf("Error generating key.\n");
		return 1;
	}
	//ft_memcpy(env->key, "aaaabbbbccccdddd\0", 17);

	print_key(env);	
	
	inject_code(env);

	// encrypt .text
	if (rabbit_encrypt(env, env->key))						//ENCRYPT
	{
		printf("Error encrypting elf.\n");
		return 1;
	}
	
	// save new obj
	dump_obj(env);

	return 0;
}

static void woody_woodpacker(void *obj, size_t size)
{
	t_env 	*env;

	// create env
	if ((env = (t_env *)malloc(sizeof(t_env))) == NULL)
	{
		return;
	}
	env->obj = obj;
	env->obj_cpy = NULL;
	env->obj_size = size;
	env->new_obj_size = size;
	env->obj_base = 0;
	env->payload_content = NULL;
	env->payload_size = 0;
	env->payload_rodata_size = 0;
	env->found_code_cave = 0;
	env->encrypt_size = 0;
	env->text_addr = NULL;
	env->bss_offset = 0;
	env->bss_size = 0;
	env->load_align = 0;
	env->entrypoint = 0;
	env->inject_offset = 0;
	env->inject_addr = 0;
	env->inject_dist = 0;
	env->page_offset = 0;
	if (((char*)obj)[5] == 1)
		env->cpu = 1;
	else
		env->cpu = 0;
	handle_obj(env);
	clear_env(env);
}

static void	read_obj(char *obj_name)
{
	int	fd;
	void *obj;
	size_t size;

	if ((fd = open(obj_name, O_RDONLY)) < 0)
	{
		print_err("Error openning file", obj_name);
		return ;
	}
	if ((size = lseek(fd, (size_t)0, SEEK_END)) <= 0)
	{
		print_err("Error empty file", obj_name);
		close(fd);
		return ;
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
		if (check_corruption(obj, size, obj_name) == 0)
		{
			woody_woodpacker(obj, size);
		}
		if (munmap(obj, size) < 0)
			print_err("Error munmap", "");
	}
}

int main(int argc, char **argv)
{
	if (argc == 2)
		read_obj(argv[1]);
	else if (argc == 1)
		read_obj("a.out\0");
	else {
		printf("Error: Too many args...\n");
	}
	return (0);
}