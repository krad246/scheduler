#ifndef CLOCKS_H_
#define CLOCKS_H_

#include <msp430.h>
#include <stdint.h>

#include <task.h>
#include <kmath.h>
#include <config.h>

#ifdef F_CPU
template <uint32_t frequency>
struct Clock {
	struct state {
		static constexpr uint16_t vloFreq = 12000;
		static constexpr uint16_t ticksPerOverflow = frequency == 1000000 ? 512 : 8192;

		static constexpr uint16_t millisInc = (state::ticksPerOverflow / (frequency / 1000000)) / 1000;
		static constexpr uint16_t fractInc = (state::ticksPerOverflow / (frequency / 1000000)) % 1000;

		static uint16_t sMillisInc;
		static uint16_t sFractInc;

		static uint32_t overflows;
		static uint32_t millis;
		static uint16_t frac;

		static bool sleeping;
	};

	static void initCrystalOscillator(void) {
		// LFXT can take up to 1000ms to start.
		// Go to the loop below 4 times for a total of 2 sec timout.
		// If a timeout happens due to no XTAL present or a faulty XTAL
		// set ACLK source as VLO (~12kHz)
		uint16_t timeout = 4;
		do {
			// Clear oscillator fault flags
			BCSCTL3 &= ~LFXT1OF;

			// Clear the oscillator fault interrupt flag
			IFG1 &= ~OFIFG;

			// Delay for 500 ms
			__delay_cycles(500000 * (frequency / 1000000));

			timeout--;
			if (!timeout) break;
		} while (IFG1 & OFIFG);	// Test the fault flag

		// If starting the XTAL timed out then fall back to VLO
		if (!timeout) {
			// No XTAL present. Default P2.6/7 to GPIO
			P2SEL &= ~(BIT6 | BIT7);

			// Source ACLK from VLO
			BCSCTL3 |= LFXT1S_2;
		}
	}

	static void initClocks(void) {
		// Stop watchdog timer
		WDTCTL = WDTPW | WDTHOLD;

		// Check that the passed in frequency is valid
		constexpr bool freqValid = frequency == 1000000 | frequency == 8000000 | frequency == 12000000 | frequency == 16000000;
		static_assert(freqValid, "Frequency must be 1, 8, 12, or 16 MHz.");

		// Set the processor clock registers
		switch (frequency) {
		case 1000000:
			DCOCTL = CALDCO_1MHZ;
			BCSCTL1 = CALBC1_1MHZ;
			break;
		case 8000000:
			DCOCTL = CALDCO_8MHZ;
			BCSCTL1 = CALBC1_8MHZ;
			break;
		case 12000000:
			DCOCTL = CALDCO_12MHZ;
			BCSCTL1 = CALBC1_12MHZ;
			break;
		case 16000000:
			DCOCTL = CALDCO_16MHZ;
			BCSCTL1 = CALBC1_16MHZ;
			break;
		}

		initCrystalOscillator();

		// Set up the watchdog timer, enabling interrupts
		if (frequency == 1000000) WDTCTL = WDT_MDLY_0_5;
		else WDTCTL = WDT_MDLY_8;

		IE1 |= WDTIE;
		kstack = (uint16_t *) _get_SP_register();
		_low_power_mode_0();
	}

	static inline uint16_t *getKStack() {
		return kstack;
	}

	static inline uint32_t micros() {
		uint16_t sr = _get_interrupt_state();
		_disable_interrupt();

		const uint64_t ticks = state::overflows;

		_set_interrupt_state(sr);

		constexpr uint16_t millisPerTick = (state::ticksPerOverflow / (frequency / 1000000));
		return mul(millisPerTick, ticks);
	}

	static inline uint32_t millis() {
		uint16_t sr = _get_interrupt_state();
		_disable_interrupt();

		const uint64_t millis = state::millis;

		_set_interrupt_state(sr);

		return millis;
	}

	static inline void delayMicros(uint16_t us);
	static void sleepSeconds(uint16_t sec);
	static void sleep(uint16_t ms);
	static inline void suspend(void);
	static inline void delay(uint16_t ms);

private:
	static uint16_t *kstack;
};

using cl = Clock<F_CPU>;

#endif

#endif /* CLOCKS_H_ */
