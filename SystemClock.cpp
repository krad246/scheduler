/*
 * SystemClock.cpp
 *
 *  Created on: Apr 5, 2019
 *      Author: krad2
 */

#include <SystemClock.h>

std::size_t SystemClock::microsPerTick = 0;
std::size_t SystemClock::micros = 0;
std::size_t SystemClock::fractionalMillis = 0;
std::size_t SystemClock::millis = 0;

void SystemClock::StartSystemClock(std::size_t frequency) {
	switch (frequency) {
	case (std::size_t) 1000000:
		DCOCTL = CALDCO_1MHZ;
		BCSCTL1 = CALBC1_1MHZ;
		break;
	case (std::size_t) 8000000:
		DCOCTL = CALDCO_8MHZ;
		BCSCTL1 = CALBC1_8MHZ;
		break;
	case (std::size_t) 12000000:
		DCOCTL = CALDCO_12MHZ;
		BCSCTL1 = CALBC1_12MHZ;
		break;
	case (std::size_t) 16000000:
		DCOCTL = CALDCO_16MHZ;
		BCSCTL1 = CALBC1_16MHZ;
		break;
	}

	SystemClock::microsPerTick = frequency == 1000000 ? 512 : 8192;

	if (frequency == 1000000) WDTCTL = WDT_MDLY_0_5;
	else WDTCTL = WDT_MDLY_8;
	IE1 |= WDTIE;


}

void SystemClock::StartCrystalOscillator(std::size_t frequency) {
	std::uint16_t timeout = 4;
	do {
		BCSCTL3 &= ~LFXT1OF;
		IFG1 &= ~OFIFG;

		switch (frequency) {
		case (std::size_t) 1000000:
			_delay_cycles(500000);
			break;
		case (std::size_t) 8000000:
			_delay_cycles(4000000);
			break;
		case (std::size_t) 12000000:
			_delay_cycles(6000000);
			break;
		case (std::size_t) 16000000:
			_delay_cycles(8000000);
			break;
		}

		timeout--;
		if (!timeout) break;
	} while (IFG1 & OFIFG);

	if (!timeout) {
		P2SEL &= ~(BIT6 | BIT7);
		BCSCTL3 |= LFXT1S_2;
	}
}
