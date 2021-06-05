/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   encrypt.c                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: eparisot <eparisot@42.student.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2021/02/26 14:55:42 by eparisot          #+#    #+#             */
/*   Updated: 2021/02/26 14:55:42 by eparisot         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../includes/woody_woodpacker.h"

#define LSW(n) (n << 8) >> 8
#define MSW(n) (n >> 8) << 8
#define FG(a, b) LSW(((a + b) % WORDSIZE) * ((a + b) % WORDSIZE)) ^ MSW(((a + b) % WORDSIZE) * ((a + b) % WORDSIZE))

static void rabbit_round(unsigned int *C, unsigned int *A, unsigned int *G, unsigned int *X, unsigned int *b)
{
	// Counter System
	for (int i = 0; i < 8; ++i)
	{
		unsigned int temp = C[i] + A[i] + *b;
		*b = temp / WORDSIZE;
		C[i] = temp % WORDSIZE;
	}

	// Next-State Function
	for (int i = 0; i < 4; ++i)
	{
		for (int j = 0; j < 8; ++j)
			G[j] = FG(X[j], C[j]);

		X[0] = G[0] + (G[7] << 16) + (G[6] << 16) % WORDSIZE;
		X[1] = G[1] + (G[0] <<  8) +  G[7]        % WORDSIZE;
		X[2] = G[2] + (G[1] << 16) + (G[0] << 16) % WORDSIZE;
		X[3] = G[3] + (G[2] <<  8) +  G[1]        % WORDSIZE;
		X[4] = G[4] + (G[3] << 16) + (G[2] << 16) % WORDSIZE;
		X[5] = G[5] + (G[4] <<  8) +  G[3]        % WORDSIZE;
		X[6] = G[6] + (G[5] << 16) + (G[4] << 16) % WORDSIZE;
		X[7] = G[7] + (G[6] <<  8) +  G[5]        % WORDSIZE;
	}
}

int rabbit_encrypt(t_env *env, unsigned char *key)
{
	unsigned int 	X[8];
	unsigned int 	C[8];
	short			K[16];
	unsigned int 	G[8];
	unsigned int 	A[8];
	unsigned int 	b = 0;
	short			S[8];

	ft_bzero(X, 8 * sizeof(unsigned int));
	ft_bzero(C, 8 * sizeof(unsigned int));
	ft_bzero(K, 8 * sizeof(short));
	ft_bzero(G, 8 * sizeof(unsigned int));
	ft_bzero(A, 8 * sizeof(unsigned int));

	// init K
	for (int i = 0; i < 16; ++i)
	{
		if (i % 2 == 0)
			K[i/2] = (key[i] << 8) + key[i+1];
	}

	// Key Setup Scheme
	for (int i = 0; i < 8; ++i)
	{
		if (i % 2 == 0)
		{
			X[i] = K[(i + 1) % 8] | K[i];
			C[i] = K[(i + 4) % 8] | K[(i + 5) % 8];
		}
		else
		{
			X[i] = K[(i + 5) % 8] | K[(i + 4) % 8];
			C[i] = K[i] | K[(i + 1) % 8];
		}
	}

	// intit A
	A[0] = 0x4D34D34D;
	A[1] = 0xD34D34D3;
	A[2] = 0x34D34D34;
	A[3] = 0x4D34D34D;
	A[4] = 0xD34D34D3;
	A[5] = 0x34D34D34;
	A[6] = 0x4D34D34D;
	A[7] = 0xD34D34D3;

	// init KEY loop
	for (int i = 0; i < 4; ++i)
	{
		rabbit_round(C, A, G, X, &b);
	}

	// main loop
	int done = 0;
	int str_c = 0;
	while (!done)
	{
		S[0] = ((short *)X)[0] ^ ((short *)X)[11];
		S[1] = ((short *)X)[1] ^ ((short *)X)[6];
		S[2] = ((short *)X)[4] ^ ((short *)X)[15];
		S[3] = ((short *)X)[5] ^ ((short *)X)[10];
		S[4] = ((short *)X)[8] ^ ((short *)X)[3];
		S[5] = ((short *)X)[9] ^ ((short *)X)[14];
		S[6] = ((short *)X)[12] ^ ((short *)X)[7];
		S[7] = ((short *)X)[13] ^ ((short *)X)[2];

		// encrypt
		int i = 0;
		for (i = 0; i < 16; ++i)
		{
			if (str_c + i < (int)env->encrypt_size)
			{
				env->text_addr[str_c + i] ^= ((char*)S)[i];
			}
			else
			{
				done = 1;
				break;
			}
		}
		str_c += i;
		
		rabbit_round(C, A, G, X, &b);
		
	}
	return 0;
}