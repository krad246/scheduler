#include <depends.h>

static volatile std::uint32_t p = 0;
static volatile std::uint32_t q = 0;

void *foo(void *arg) {
	while (1) {
		p++;
	}
	return 0;
}

void *bar(void *arg) {
	while (1) {
		q++;
	}
	return 0;
}

int main(void) {
	WDTCTL = WDTPW | WDTHOLD;	// stop watchdog timer
	TaskQueue x;
	x.addTask(foo, 7);
	x.addTask(bar);

	Scheduler sched(x);
	sched.start();

	_low_power_mode_0();
	return 0;
}
