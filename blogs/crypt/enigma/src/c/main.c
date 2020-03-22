#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "enigma.h"

void connect_plugboard(struct enigma *machine)
{
	enigma_add_connect(machine, 'A', 'X');
	enigma_add_connect(machine, 'D', 'I');
	enigma_add_connect(machine, 'H', 'P');
	enigma_add_connect(machine, 'R', 'W');
	enigma_add_connect(machine, 'S', 'Z');
	enigma_add_connect(machine, 'O', 'K');
}

void output_map_status(int *map)
{
	for (int i = 0; i < KEY_NUM; ++i)
	{
		printf("%c", (char)i + 'A');
	}
	printf("\n");
	for (int i = 0; i < KEY_NUM; ++i)
	{
		printf("%c", (char)map[i] + 'A');
	}
	printf("\n");
}

void output_rotor_status(struct rotor *r, int reverse)
{
	int offset = r->offset;
	for (int i = 0; i < 26; ++i)
	{
		printf("%c", (char)i + 'A');
	}
	printf("\n");
	for (int i = 0; i < 26; ++i)
	{
		if (reverse)
		{
			int idx = r->rmap[i] - offset;
			if (idx < 0)
			{
				idx += 26;
			}
			printf("%c", (char)idx + 'A');
		}
		else
		{
			int idx = (i + offset) % 26;
			printf("%c", (char)r->map[idx] + 'A');
		}
	}
	printf("\n");
}

void output_enigma_status(struct enigma *machine)
{
	printf("plugboard\n");
	output_map_status(machine->pb.map);

	for (int i = 0; i < SLOT_NUM; ++i)
	{
		printf("rotors[%d]\n", i);
		output_rotor_status(machine->rotors[i], 0);
	}

	printf("reflector:\n");
	output_map_status(machine->r.map);

	for (int i = SLOT_NUM - 1; i >= 0; --i)
	{
		printf("reverse rotors[%d]\n", i);
		output_rotor_status(machine->rotors[i], 1);
	}

	printf("reverse plugboard:\n");
	output_map_status(machine->pb.map);
}

int main(int argc, char *argv[])
{
	if (argc < 3)
	{
		fprintf(stderr, "usage: %s communicate_key(three random alphabet, two times) plaintext\n", argv[0]);
		exit(EXIT_FAILURE);
	}

	const char *communicate_key = argv[1];
	const char *plaintext = argv[2];

	// enigma machine
	struct enigma machine;
	memset(&machine, 0, sizeof(machine));
	for (int i = 0; i < KEY_NUM; ++i)
	{
		machine.pb.map[i] = i;
		machine.r.map[i] = i;
	}

	// srand(time(NULL));
	srand(0);

	// create reflector
	enigma_gen_reflector_rand(&machine.r);

	// create rotors
	struct rotor rotors[SLOT_NUM];
	for (int i = 0; i < SLOT_NUM; ++i)
	{
		enigma_gen_rotor_rand(&rotors[i]);
	}

	printf("Enigma machine components\n");
	printf("reflect: ");
	enigma_output_reflect(&machine.r);
	for (int i = 0; i < SLOT_NUM; ++i)
	{
		printf("rotor[%d]: ", i);
		enigma_output_rotor(&rotors[i]);
	}

    ////////////////////////////////////////////
	// today daily key
	// plugboard: A<->X D<->I H<->P R<->W S<->Z O<->K
	// rotor orders: 1,2,0
	// rotor init rotate: Q-C-W
	printf("\nEnigma today's daily key\n");
	printf("plugboard: A<->X D<->I H<->P R<->W S<->Z O<->K\n");
	connect_plugboard(&machine);
	printf("rotor orders: 1,2,0\n");
	enigma_put_rotors(&machine, 0, &rotors[1]);
	enigma_put_rotors(&machine, 1, &rotors[2]);
	enigma_put_rotors(&machine, 2, &rotors[0]);
	printf("rotor init rotate: Q-C-W\n");
	enigma_reset_rotor_rotation(&machine, 0, 'Q');
	enigma_reset_rotor_rotation(&machine, 1, 'C');
	enigma_reset_rotor_rotation(&machine, 2, 'W');
	printf("\n");

#if MUGGLE_BUILD_TRACE
	output_enigma_status(&machine);
#endif

    ////////////////////////////////////////////
	// encrypt
	size_t communicate_key_len = strlen(communicate_key);
	size_t text_len = strlen(plaintext);
	char *ciphertext = (char*)malloc(communicate_key_len + text_len + 1);
	size_t idx = 0, n;

	if (communicate_key_len != 6)
	{
		fprintf(stderr, "in this example, assume 3 rotors in machine, communicate_key must set 3 alphas");
		exit(EXIT_FAILURE);
	}

	// KEK
	for (size_t i = 0; i < communicate_key_len; ++i)
	{
		ciphertext[idx++] = enigma_input(&machine, communicate_key[i]);
#if MUGGLE_BUILD_TRACE
		output_enigma_status(&machine);
#endif
	}

	// reset rotor
	enigma_reset_rotor_rotation(&machine, 0, communicate_key[0]);
	enigma_reset_rotor_rotation(&machine, 1, communicate_key[1]);
	enigma_reset_rotor_rotation(&machine, 2, communicate_key[2]);

	// input plaintext
	for (size_t i = 0; i < text_len; ++i)
	{
		ciphertext[idx++] = enigma_input(&machine, plaintext[i]);
	}
	ciphertext[idx] = '\0';
	n = idx + 1;

	printf("plaintext: (key)%s ", communicate_key);
	printf("%s\n", plaintext);

	printf("ciphertext: ");
	printf("%s\n", ciphertext);


    ////////////////////////////////////////////
	// decrypt
	enigma_reset_rotor_rotation(&machine, 0, 'Q');
	enigma_reset_rotor_rotation(&machine, 1, 'C');
	enigma_reset_rotor_rotation(&machine, 2, 'W');

#if MUGGLE_BUILD_TRACE
	output_enigma_status(&machine);
#endif

	char *ret_plaintext = (char*)malloc(n);
	idx = 0;
	for (size_t i = 0; i < communicate_key_len; ++i)
	{
		ret_plaintext[idx++] = enigma_input(&machine, ciphertext[i]);
#if MUGGLE_BUILD_TRACE
		output_enigma_status(&machine);
#endif

	}

	for (size_t i = 0; i < idx / 2; ++i)
	{
		if (ret_plaintext[i] != ret_plaintext[i + idx / 2])
		{
			ret_plaintext[idx] = '\0';
			fprintf(stderr, "failed decrypt communicate key %s\n", ret_plaintext);
			exit(EXIT_FAILURE);
		}
	}

	enigma_reset_rotor_rotation(&machine, 0, ret_plaintext[0]);
	enigma_reset_rotor_rotation(&machine, 1, ret_plaintext[1]);
	enigma_reset_rotor_rotation(&machine, 2, ret_plaintext[2]);

	for (size_t i = idx; i < n - 1; ++i)
	{
		ret_plaintext[idx++] = enigma_input(&machine, ciphertext[i]);
	}
	ret_plaintext[idx] = '\0';

	printf("decrpyt: ");
	printf("(key)%.6s %s\n", ret_plaintext, &ret_plaintext[6]);
	
	// free
	free(ciphertext);
	free(ret_plaintext);

	return 0;
}
