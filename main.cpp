#include <depends.h>

static volatile std::uint32_t x = 0;
static volatile std::uint32_t xold = 0;
static volatile std::uint32_t y = 0;
static volatile std::uint32_t yold = 0;
static volatile std::uint32_t z = 0;
static volatile std::uint32_t zold = 0;

void foo(void) {
	while (1) {
		Scheduler::sleep(1);
		x++;
	}
}

void bar(void) {
	while (1) {
		Scheduler::sleep(1);
		y++;

	}
}

void baz(void) {
	while (1) {
		Scheduler::sleep(1);
		z++;
	}
}

int main(void) {
	WDTCTL = WDTPW | WDTHOLD;	// stop watchdog timer
	TaskQueue x;
	x.addTask(foo);
	x.addTask(bar);
	x.addTask(baz);
	Scheduler s(x, Scheduler::roundRobin);
	P1DIR = BIT0 | BIT6;
	s.start((std::size_t) 16000000);
	_low_power_mode_0();
	return 0;
}
