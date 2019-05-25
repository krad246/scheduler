/*
 * sys.h
 *
 *  Created on: May 14, 2019
 *      Author: krad2
 */

#ifndef CONTEXT_H_
#define CONTEXT_H_

#define __USEMSP430X__

#if defined (__MSP430__)
	#if defined (__MSP430X__)
	#else
		#undef __USEMSP430X__
	#endif
#else
	#error
#endif

namespace sys {
	#pragma FUNC_ALWAYS_INLINE
	inline void saveContext(void) {
		#if defined (__USEMSP430X__)
			asm("  	pushm.a #12, r15\n");
		#else
			asm( \
				"   push r4\n" \
				"   push r5\n" \
				"   push r6\n" \
				"   push r7\n" \
				"   push r8\n" \
				"   push r9\n" \
				"   push r10\n" \
				"   push r11\n" \
				"   push r12\n" \
				"   push r13\n" \
				"   push r14\n" \
				"   push r15\n" \
			);
		#endif
	}

	#pragma FUNC_ALWAYS_INLINE
	inline void restoreContext(void) {
		#if defined (__USEMSP430X__)
			asm("   popm.a #12, r15\n" \
				"   pop.w r2\n" \
				"   movx.b 2(sp), r15\n" \
				"   movx.a sp, r14\n" \
				"   movx.b -1(sp), r13\n" \
				"   rram.a #4, r13\n" \
				"   movx.b r13, 2(sp)\n" \
				"   movx.a @sp, r5\n" \
				"   movx.b r15, 2(r14)\n" \
				"   movx.a r4, sp\n" \
				"   bra r5\n" \
			);


		#else
			asm( \
				"   pop r4\n" \
				"   pop r5\n" \
				"   pop r6\n" \
				"   pop r7\n" \
				"   pop r8\n" \
				"   pop r9\n" \
				"   pop r10\n" \
				"   pop r11\n" \
				"   pop r12\n" \
				"   pop r13\n" \
				"   pop r14\n" \
				"   pop r15\n" \
				"   pop r2\n" \
				"   mov.w @sp, r5\n" \
				"   mov.w r4, sp\n" \
				"   br r5\n" \
			);
		#endif
	}
};

#endif /* CONTEXT_H_ */
