#include "clocks.h"

template <uint32_t frequency>
uint32_t Clock<frequency>::state::millis = 0;

template <uint32_t frequency>
uint16_t Clock<frequency>::state::frac = 0;

template <uint32_t frequency>
uint32_t Clock<frequency>::state::overflows = 0;

template <uint32_t frequency>
uint16_t Clock<frequency>::state::sMillisInc = 0;

template <uint32_t frequency>
uint16_t Clock<frequency>::state::sFractInc = 0;

template <uint32_t frequency>
bool Clock<frequency>::state::sleeping = false;

template <uint32_t frequency>
uint16_t *Clock<frequency>::kstack = 0;

#pragma vector = WDT_VECTOR
interrupt void watchdogIsr(void) {
	// Fight compiler calling convention
	asm volatile (
			"   pop r11 \n" \
			"   pop r12 \n" \
			"	pop r13 \n" \
			"	pop r14 \n" \
			"	pop r15 \n" \
	);

	saveContext();

	register uint32_t millis = cl::state::millis;
	register uint16_t fracs = cl::state::frac;

	millis += cl::state::sleeping ? cl::state::sMillisInc : cl::state::millisInc;
	fracs += cl::state::sleeping ? cl::state::sFractInc : cl::state::fractInc;

	if (fracs >= 1000) {
		fracs -= 1000;
		millis += 1;
	}

	cl::state::frac = fracs;
	cl::state::millis = millis;
	cl::state::overflows++;

	// Pick new task to run, do the work on the kernel stack
	_set_SP_register((uint16_t) cl::getKStack());
	volatile const Task &runnable = tq::pickTask();
	_set_SP_register((uint16_t) runnable.sp);

	// Restore context
	restoreContext();

	asm volatile(" 	reti \n");
}
