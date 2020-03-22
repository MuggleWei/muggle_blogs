#include "enigma.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

int enigma_add_connect(struct enigma *p_enigma, char c1, char c2)
{
	c1 = toupper(c1);
	c2 = toupper(c2);

	if (c1 < 'A' || c1 > 'Z' || c2 < 'A' || c2 > 'Z')
	{
		fprintf(stderr, "invalid connection: %c <-> %c\n", c1, c2);
		return -1;
	}

	c1 -= 'A';
	c2 -= 'A';
	if (p_enigma->pb.map[c1] != p_enigma->pb.map[c1] ||
		p_enigma->pb.map[c2] != p_enigma->pb.map[c2])
	{
		fprintf(stderr, "invalid to add connection repeated: %c <-> %c\n", c1 + 'A', c2 + 'A');
		return -1;
	}

	p_enigma->pb.map[c1] = c2;
	p_enigma->pb.map[c2] = c1;

	return 0;
}

void enigma_gen_rotor_rand(struct rotor *p_rotor)
{
	memset(p_rotor, 0, sizeof(struct rotor));

	for (int i = 0; i < KEY_NUM; ++i)
	{
		p_rotor->map[i] = i;
		p_rotor->rmap[i] = i;
	}

	for (int i = 0; i < KEY_NUM; ++i)
	{
		int r = rand() % KEY_NUM;
		int tmp = p_rotor->map[i];
		p_rotor->map[i] = p_rotor->map[r];
		p_rotor->map[r] = tmp;
	}

	for (int i = 0; i < KEY_NUM; ++i)
	{
		p_rotor->rmap[p_rotor->map[i]] = i;
	}
}

int enigma_put_rotors(struct enigma *p_enigma, unsigned int idx, struct rotor *p_rotor)
{
	if (idx >= SLOT_NUM)
	{
		fprintf(stderr, "slot index is beyond the limit\n");
		return -1;
	}

	if (p_enigma->rotors[idx] != NULL)
	{
		fprintf(stderr, "try to put rotor into slot[%u] repeatedly", idx);
		return -1;
	}

	p_enigma->rotors[idx] = p_rotor;

	return 0;
}

int enigma_reset_rotor_rotation(struct enigma *p_enigma, unsigned int idx, char c)
{
	if (idx >= SLOT_NUM)
	{
		fprintf(stderr, "slot index is beyond the limit\n");
		return -1;
	}

	c = toupper(c);
	if (c < 'A' || c > 'Z')
	{
		fprintf(stderr, "need input alphabet");
		return -1;
	}
	unsigned int offset = (unsigned int)(c - 'A');

	p_enigma->rotors[idx]->steps = 0;
	p_enigma->rotors[idx]->offset = offset;

	return 0;
}

void enigma_gen_reflector_rand(struct reflector *ref)
{
	for (int i = 0; i < KEY_NUM; ++i)
	{
		if (ref->map[i] != i)
		{
			continue;
		}

		int r = rand() % KEY_NUM;
		int cnt_try = 0;
		while (ref->map[r] != r)
		{
			r += 1;
			if (r >= KEY_NUM)
			{
				r -= KEY_NUM;
			}
			++cnt_try;
			if (cnt_try == KEY_NUM)
			{
				return;
			}
		}
		int tmp = ref->map[i];
		ref->map[i] = ref->map[r];
		ref->map[r] = tmp;
	}
}

char enigma_input(struct enigma *p_enigma, char c)
{
#if MUGGLE_BUILD_TRACE
	printf("-------------------\n");
#endif
	c = toupper(c);
	if (c < 'A' || c > 'Z')
	{
		fprintf(stderr, "need input alphabet");
		return 0;
	}

#if MUGGLE_BUILD_TRACE
	printf("input: %c", c);
#endif
	// plugboard
	int x = c - 'A';
	x = p_enigma->pb.map[x];
#if MUGGLE_BUILD_TRACE
	printf(" -> plug -> %c", 'A' + x);
#endif

	// rotors
	for (int i = 0; i < SLOT_NUM; ++i)
	{
		x += p_enigma->rotors[i]->offset;
		if (x >= KEY_NUM)
		{
			x -= KEY_NUM;
		}
		x = p_enigma->rotors[i]->map[x];
#if MUGGLE_BUILD_TRACE
		printf(" -> rotor%d -> %c", i, 'A' + x);
#endif
	}

	// reflector
	x = p_enigma->r.map[x];
#if MUGGLE_BUILD_TRACE
	printf(" -> %c(reflect)", 'A' + x);
#endif

	// rotors
	for (int i = SLOT_NUM - 1; i >= 0; --i)
	{
		x = p_enigma->rotors[i]->rmap[x];
		x -= p_enigma->rotors[i]->offset;
		if (x < 0)
		{
			x += KEY_NUM;
		}
#if MUGGLE_BUILD_TRACE
		printf(" -> rotor%d -> %c", i, 'A' + x);
#endif
	}

	// plugboard
	x = p_enigma->pb.map[x];
#if MUGGLE_BUILD_TRACE
	printf(" -> (plug) -> %c", 'A' + x);
#endif


	// output
	c = (char)x + 'A';

	// rotate rotors
	int idx_rotor = 0;
	while (1)
	{
		struct rotor *p_rotor = p_enigma->rotors[idx_rotor]; 
		p_rotor->offset += 1;
		if (p_rotor->offset >= KEY_NUM)
		{
			p_rotor->offset  = 0;
		}

		++p_rotor->steps;
		if (p_rotor->steps >= KEY_NUM)
		{
			p_rotor->steps = 0;
			++idx_rotor;
			if (idx_rotor >= SLOT_NUM)
			{
				break;
			}
		}
		else
		{
			break;
		}
	}

#if MUGGLE_BUILD_TRACE
	printf(" | offset ");
	for (int i = 0; i < SLOT_NUM; ++i)
	{
		printf("%d ", (int)p_enigma->rotors[i]->offset);
	}
	printf("\n");
#endif

	return c;
}

void enigma_output_rotor(struct rotor *p_rotor)
{
	for (int i = 0; i < KEY_NUM; ++i)
	{
		printf("%c<->%c ", i + 'A', p_rotor->map[i] + 'A');
	}
	printf("\n");
}

void enigma_output_reflect(struct reflector *ref)
{
	for (int i = 0; i < KEY_NUM; ++i)
	{
		if (ref->map[i] >= i)
		{
			printf("%c<->%c ", i + 'A', ref->map[i] + 'A');
		}
	}
	printf("\n");
}
