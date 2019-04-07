#include <depends.h>

static volatile std::uint32_t x = 0;
static volatile std::uint32_t y = 0;
static volatile std::uint32_t yold = 0;
static volatile std::uint32_t z = 0;
static volatile std::uint32_t zold = 0;

void foo(void) {
	while (1) {
		x++;
	}
}

void bar(void) {
	while (1) {
		y++;
		if (y - yold >= 16384) {
			yold = y;
			P1OUT ^= BIT0;
		}
	}
}

void baz(void) {
	while (1) {
		z++;
		if (z - zold >= 16384) {
			zold = z;
			P1OUT ^= BIT6;
		}
	}
}

int main(void) {
	WDTCTL = WDTPW | WDTHOLD;	// stop watchdog timer
	TaskQueue x;
	x.addTask(foo);
	x.addTask(bar, 0, 5);
	x.addTask(baz, 0, 3);

	Scheduler s(x, Scheduler::lottery);
	P1DIR = BIT0 | BIT6;
	s.start((std::size_t) 8000000);
	_low_power_mode_0();
	return 0;
}
