#include <depends.h>

static volatile std::uint32_t p = 0;
static volatile std::uint32_t q = 0;

void *foo(void *arg) {
	while (1) {
		p++;
		if (p >= 2048) {
			p -= 2048;
			P1OUT ^= BIT0;
		}
	}
	return 0;
}

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

int main(void) {
	WDTCTL = WDTPW | WDTHOLD;	// stop watchdog timer
	TaskQueue x;
	x.addTask(foo, 7);
	x.addTask(bar);
	P1DIR |= (BIT0 | BIT6);
	Scheduler sched(x);
	sched.start(8000000);

	_low_power_mode_0();
	return 0;
}
