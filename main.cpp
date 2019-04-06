#include <depends.h>

static volatile std::uint32_t p = 0;
static volatile std::uint32_t pold = 0;
static volatile std::uint32_t q = 0;
static volatile std::uint32_t z = 0;

void *bar(void *arg) {
	while (1) {
		q++;
		if (q >= 1024) {
			q -= 1024;
			P1OUT ^= BIT6;
		}
	}
	return 0;
}

void *baz(void *arg) {
	while (1) {
		z++;
	}
}


TaskQueue x;
void *foo(void *arg) {
	while (p <= 32768) {
		p++;
		if (p - pold >= 2048) {
			pold = p;
			P1OUT ^= BIT0;
		}
	}
//	x.addTask(baz);
	return 0;
}

int main(void) {
	WDTCTL = WDTPW | WDTHOLD;	// stop watchdog timer
	x.addTask(foo, 7);
	x.addTask(bar);
	P1DIR |= (BIT0 | BIT6);
	Scheduler sched(x);
	sched.start(1000000);

	_low_power_mode_0();
	return 0;
}
