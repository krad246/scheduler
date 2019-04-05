#include <kmath.h>

uint64_t mul(uint32_t x, uint32_t y) {
	register uint64_t ans = 0;

	register uint32_t smallerNum = x > y ? y : x;
	const register uint32_t largerNum = x > y ? x : y;

	uint8_t count = 0;
	while (smallerNum) {
		if (smallerNum & 1) {
			ans += largerNum << count;
		}

		count++;
		smallerNum >>= 1;
	}

	return ans;
}

uint16_t mod(uint16_t a, uint16_t b) {
	uint16_t k = 0;

	while (a - mul(k, b) >= b) {
		++k;
	}

	return a - mul(k, b);
}
