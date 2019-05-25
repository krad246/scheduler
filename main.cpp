<<<<<<< HEAD
#include <depends.h>

static volatile std::uint32_t x = 0;
static volatile std::uint32_t xold = 0;
static volatile std::uint32_t y = 0;
static volatile std::uint32_t yold = 0;
static volatile std::uint32_t z = 0;
static volatile std::uint32_t zold = 0;

void foo(void) {
	while (1) {
		volatile int q = 0;
		q = 0xBEEF;
		volatile int p = 0;
		p++;
		x++;
		p++;
	}
}

void bar(void) {
	while (1) {
		y++;

	}
}

void baz(void) {
	while (1) {
		z++;
	}
=======
#include <msp430.h> 
#include <task.h>
#include <vector>
#include <scheduler.h>
#include <cstdint>
void foo(int*x){
	*x = *x + 1;
}

volatile std::uint32_t j = 0;
volatile std::uint32_t k = 0;

std::uint16_t bar(void *y) {
	while (1) {
		j++;
	}
	return 0;
}

std::uint16_t baz(void *x) {
	while (1) {
		k++;
	}
	return 0;
>>>>>>> master
}

int main(void) {
	WDTCTL = WDTPW | WDTHOLD;	// stop watchdog timer
<<<<<<< HEAD
	TaskQueue x;
	x.addTask(foo, 6, 1);
	x.addTask(bar, 20, 5);
	x.addTask(baz, 2, 3);
	Scheduler s(x, Scheduler::lottery);
	P1DIR = BIT0 | BIT6;
	s.start((std::size_t) 16000000);
	_low_power_mode_0();
=======
	scheduler<schedulerBase::lottery> p{task<bar, 1>(), task<baz, 3>()};


	WDTCTL = WDT_MDLY_8;

	SFRIE1 |= WDTIE;
	_disable_interrupts();

	p.start();

	_low_power_mode_1();
>>>>>>> master
	return 0;
}
