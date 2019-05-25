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
}

int main(void) {
	WDTCTL = WDTPW | WDTHOLD;	// stop watchdog timer
	scheduler<schedulerBase::lottery> p{task<bar, 1>(), task<baz, 3>()};


	WDTCTL = WDT_MDLY_8;

	SFRIE1 |= WDTIE;
	_disable_interrupts();

	p.start();

	_low_power_mode_1();
	return 0;
}
