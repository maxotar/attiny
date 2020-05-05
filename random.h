#ifndef RANDOM_H_
#define RANDOM_H_

static uint8_t random_value = 1;

void set_seed(uint8_t seed) {
	random_value = seed;
}

uint8_t random_0_255(void) {
	// https://codebase64.org/doku.php?id=base:small_fast_8-bit_prng
	// Returns values from 0 to 255 inclusive, period is 256
	// other magic values = 29, 43, 45 77, 95, 99, 101, 105, 113, 135, 141, 169, 195, 207, 231, 245
	const uint8_t magic_val = 29;
	
	if (random_value == 0)
	{
		random_value ^= magic_val;
		return random_value;
	}
	
	if (random_value == 0x80)
	{
		random_value <<= 1;
		return random_value;
	}

	if (random_value & 0x80)
	{
		random_value <<= 1;
		random_value ^= magic_val;
		return random_value;
	}

	random_value <<= 1;
	return random_value;
}

#endif /* RANDOM_H_ */