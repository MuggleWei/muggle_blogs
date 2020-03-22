#ifndef ENIGMA_H_
#define ENIGMA_H_

#ifdef __cplusplus
extern "C" {
#endif // #ifdef __cplusplus

#define KEY_NUM  26
#define SLOT_NUM 3

struct plugboard
{
	int map[KEY_NUM];
};

struct rotor
{
	int map[KEY_NUM];
	int rmap[KEY_NUM];
	unsigned int steps;
	unsigned int offset;
};

struct reflector
{
	int map[KEY_NUM];
};

struct enigma
{
	struct plugboard pb;
	struct rotor     *rotors[SLOT_NUM];
	struct reflector r;
};

/*
 * add wiring in plugboard
 * @p_enigma: enigma machine
 * @x1, x2: the alphabet need to connected
 * RETURN: 0 - success, otherwise - failed
 * */
int enigma_add_connect(struct enigma *p_enigma, char x1, char x2);

/*
 * generate rotor randomly
 * */
void enigma_gen_rotor_rand(struct rotor *p_rotor);

/*
 * put rotor into enigma machine
 * @p_enigma: enigma machine
 * @idx: index of rotor slot
 * @p_rotor: enigma rotor
 * RETURN: 0 - success, otherwise - failed
 * */
int enigma_put_rotors(struct enigma *p_enigma, unsigned int idx, struct rotor *p_rotor);

/*
 * reset rotor init rotation
 * @p_enigma: enigma machine
 * @idx: index of rotor slot
 * @c: an alphabet
 * RETURN: 0 - success, otherwise - failed
 * */
int enigma_reset_rotor_rotation(struct enigma *p_enigma, unsigned int idx, char c);

/*
 * generate reflector randomly
 * */
void enigma_gen_reflector_rand(struct reflector *r);

/*
 * enigma input
 * @p_enigma: enigma machine
 * @c: an alphabet
 * RETURN: output an alphabet, if failed then return 0
 * */
char enigma_input(struct enigma *p_enigma, char c);

/*
 * output rotor
 * */
void enigma_output_rotor(struct rotor *p_rotor);

/*
 * output reflector
 * */
void enigma_output_reflect(struct reflector *ref);


#ifdef __cplusplus
}
#endif // #ifdef __cplusplus

#endif // #ifndef ENIGMA_H_
