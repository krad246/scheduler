/*
 * task.cpp
 *
 *  Created on: May 14, 2019
 *      Author: krad2
 */

#ifdef TASK_H_

std::uint16_t taskBase::idle(void *arg) {
	(void) arg;
	_low_power_mode_0();
	return 0;
}

template <runnable r, std::size_t priorityVal, std::size_t stackSize>
task<r, priorityVal, stackSize>::task() {
	static_assert(priorityVal >= 1, "Task priority must be greater than 1");
	static_assert(stackSize >= 4, "Stack size must be a minimum of 4 bytes");

	this->func = r;

	#if defined (__USEMSP430X__)
		this->trapframe = new std::uint32_t[15];
	#else
		this->trapframe = new std::uint16_t[16];
	#endif

	this->userStack = new std::uint8_t[stackSize];

	#if defined (__USEMSP430X__)
		this->trapframe[14] = reinterpret_cast<std::uint32_t>(r);
		this->trapframe[13] = reinterpret_cast<std::uint32_t>(task::idle);

		const std::uint32_t highPcBits = reinterpret_cast<std::uint32_t>(r) & 0xF0000;
		constexpr std::uint16_t gieBits = GIE & 0x0FFF;
		const std::uint16_t srPlusPc = highPcBits >> 4 | gieBits;
		const std::uint32_t bottomPcBits = reinterpret_cast<std::uint32_t>(r) & 0xFFFF;
		const std::uint32_t stateWords = bottomPcBits << 16 | srPlusPc;
		this->trapframe[12] = reinterpret_cast<std::uint32_t>(stateWords);

		this->trapframe[11] = reinterpret_cast<std::uint32_t>(this->userStack);

		std::memset(this->trapframe, 0x0, 44);
	#else
		this->trapframe[15] = reinterpret_cast<std::uint16_t>(r);
		this->trapframe[14] = reinterpret_cast<std::uint16_t>(task::idle);
		this->trapframe[13] = reinterpret_cast<std::uint16_t>(r);
		this->trapframe[12] = GIE;
		this->trapframe[11] = reinterpret_cast<std::uint16_t>(this->userStack);

		std::memset(this->trapframe, 0x0, 22);
	#endif

	this->priority = priorityVal;
}

template <runnable r, std::size_t priorityVal, std::size_t stackSize>
task<r, priorityVal, stackSize>::~task() {
	delete[] this->trapframe;
	delete[] this->userStack;
}

#pragma FUNC_ALWAYS_INLINE
inline bool taskBase::isComplete(void) const {
	#if defined (__USEMSP430X__)
		std::uint32_t retAddress = this->trapframe[14];
		return retAddress & 0xFFFFFF00 == 0x00000000;
	#else
		std::uint16_t retAddress = this->trapframe[15];
		return retAddress & 0xFF00 == 0x0000;
	#endif
}

#pragma FUNC_ALWAYS_INLINE
inline bool taskBase::isIdle(void) const {
	#if defined (__USEMSP430X__)
		std::uint32_t retAddress = this->trapframe[14];
		return retAddress == reinterpret_cast<std::uint32_t>(taskBase::idle);
	#else
		std::uint16_t retAddress = this->trapframe[14];
		return retAddress == reinterpret_cast<std::uint16_t>(taskBase::idle);
	#endif
}

#pragma FUNC_ALWAYS_INLINE
inline bool taskBase::isSleeping(void) const {
	return this->state == taskStates::sleeping;
}

#endif
