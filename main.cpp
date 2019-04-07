#include <depends.h>

static volatile int x = 0;
static volatile int y = 0;
static volatile std::uint32_t z = 0;
static volatile std::uint32_t zold = 0;

void foo(void) {
	while (1) {
		x++;
		if (x >= 16384) {
			x -= 16384;
		}
	}
}

void bar(void) {
	while (1) {
		y++;
		if (y >= 16384) {
			y -= 16384;
			P1OUT ^= BIT6;
		}
	}
}

void baz(void) {
	while (z <= 70000) {
		z++;

		if (z - zold >= 5000) {
			zold = z;
			P1OUT ^= BIT0;
		}
	}
	P1OUT &= ~BIT0;
}

int main(void) {
	WDTCTL = WDTPW | WDTHOLD;	// stop watchdog timer
	TaskQueue x;
	x.addTask(foo);
	x.addTask(bar);
	x.addTask(baz);
	Scheduler s(x);
	P1DIR = BIT0 | BIT6;
	s.start((std::size_t) 16000000);
	_low_power_mode_0();
	return 0;
}
