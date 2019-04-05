#include "clocks.h"
#include "task.h"

static volatile uint32_t x, y = 0;

void idle1(void) {
	while (1) {
		x++;

		P1OUT ^= BIT0;
	}
}

void idle2(void) {
	while (1) {
		y++;
		P1OUT ^= BIT6;
	}
}

void idle3(void) {
	while (1);
}

int main(void) {
	tq::addTask<idle1>();
	tq::addTask<idle2>();
	tq::addTask<idle3, 29 , 10>();
	P1DIR |= BIT0 | BIT6;
	cl::initClocks();
	return 0;
}

