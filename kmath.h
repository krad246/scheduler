#ifndef KMATH_H_
#define KMATH_H_

#include <stdint.h>

uint64_t mul(uint32_t x, uint32_t y);

inline uint16_t xs16(void) {
	static uint16_t xs = 1;
    xs ^= xs << 7;
    xs ^= xs >> 9;
    xs ^= xs << 8;
    return xs;
}

inline uint32_t xs32(void) {
	static uint32_t xs = 7;
    xs ^= xs << 13;
    xs ^= xs >> 17;
    xs ^= xs << 5;
    return xs;
}

inline uint16_t nearestPow2(uint16_t x) {
	x--;
	x |= x >> 1;  // handle  2 bit numbers
	x |= x >> 2;  // handle  4 bit numbers
	x |= x >> 4;  // handle  8 bit numbers
	x |= x >> 8;  // handle 16 bit numbers
	x++;
	return x;
}

uint16_t mod(uint16_t a, uint16_t b);

#endif /* KMATH_H_ */
