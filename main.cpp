#include <depends.h>

static volatile std::uint32_t x = 0;
static volatile std::uint32_t xold = 0;
static volatile std::uint32_t y = 0;
static volatile std::uint32_t yold = 0;
static volatile std::uint32_t z = 0;
static volatile std::uint32_t zold = 0;

void foo(void) {
	while (1) {
		x++;

		if (x >= 16384) {
			xold = x;
			P1OUT ^= BIT0;
		}
	}
}

void bar(void) {
	while (1) {
		y++;

		if (y >= 16384) {
			yold = y;
			P1OUT ^= BIT6;
		}
	}
}

void baz(void) {
	while (1) {
		z++;

		if (z >= 16384) {
			zold = z;
			P1OUT &= ~(BIT0 | BIT6);
		}
	}
}

int main(void) {
	WDTCTL = WDTPW | WDTHOLD;	// stop watchdog timer
	TaskQueue x;
	x.addTask(foo);
	x.addTask(bar, 0, 5);
	x.addTask(baz, 0, 3);

	Scheduler s(x);
	P1DIR = BIT0 | BIT6;
	s.start((std::size_t) 16000000);
	_low_power_mode_0();
	return 0;
}
