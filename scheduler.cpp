/*
 * scheduler.cpp
 *
 *  Created on: May 14, 2019
 *      Author: krad2
 */

#ifdef SCHEDULER_H_

std::size_t schedulerBase::currProc = 1;
std::vector<taskBase> schedulerBase::taskList = std::vector<taskBase>();
std::size_t schedulerBase::numSleeping = 0;
schedulingMethod schedulerBase::method = nullptr;

#if defined (__USEMSP430X__)
	std::uint32_t schedulerBase::sysSp = 0;
#else
	std::uint16_t schedulerBase::sysSp = 0;
#endif

template <schedulingMethod m>
constexpr scheduler<m>::scheduler(const std::initializer_list<taskBase>& tasks) {
	task<taskBase::idle> idleTask;
	schedulerBase::taskList.push_back(idleTask);
	for (auto i = begin(tasks); i < end(tasks); ++i) schedulerBase::taskList.push_back(*i);
	schedulerBase::method = m;
}

void schedulerBase::start(void) {
	schedulerBase::sysSp = _get_SP_register();

	const taskBase &nextTask = schedulerBase::taskList[schedulerBase::currProc];
	#if defined(__USEMSP430X__)
		_set_SP_register(reinterpret_cast<std::uint32_t>(nextTask.trapframe));
	#else
		set_SP_register(reinterpret_cast<std::uint16_t>(nextTask.trapframe));
	#endif

	sys::restoreContext();
}

const taskBase schedulerBase::roundRobin(void) {
	static std::size_t numTimesRun = 0;
	const std::size_t sz = schedulerBase::taskList.size();

	if (schedulerBase::numSleeping != sz - 1) {
		while (1) {
			const taskBase &t = schedulerBase::taskList[schedulerBase::currProc];
			if (t.isSleeping()) schedulerBase::currProc++;
			else {
				const bool taskRunEnoughTimes = (numTimesRun >= t.priority);

				if (taskRunEnoughTimes) {
					numTimesRun = 0;
					schedulerBase::currProc++;
					if (schedulerBase::currProc == sz) {
						schedulerBase::currProc = (schedulerBase::numSleeping != sz - 1);
					}
				}

				numTimesRun++;
				return schedulerBase::taskList[schedulerBase::currProc];
			}
		}
	}

	return schedulerBase::taskList[0];
}

#include <type_traits>

template <class T> struct tag {
	using type = T;
};

template <class T> struct NextSize;

template <> struct NextSize<std::uint8_t>  : tag<std::uint16_t> { };
template <> struct NextSize<std::uint16_t> : tag<std::uint32_t> { };
template <> struct NextSize<std::uint32_t> : tag<std::uint64_t> { };

template <> struct NextSize<std::int8_t>  : tag<std::int16_t> { };
template <> struct NextSize<std::int16_t> : tag<std::int32_t> { };
template <> struct NextSize<std::int32_t> : tag<std::int64_t> { };

template <std::size_t bits, bool sign> struct MinSizeType;
template <> struct MinSizeType<8, false> : tag<std::uint8_t> { };
template <> struct MinSizeType<16, false> : tag<std::uint16_t> { };
template <> struct MinSizeType<32, false> : tag<std::uint32_t> { };
template <> struct MinSizeType<64, false> : tag<std::uint64_t> { };

template <> struct MinSizeType<8, true> : tag<std::int8_t> { };
template <> struct MinSizeType<16, true> : tag<std::int16_t> { };
template <> struct MinSizeType<32, true> : tag<std::int32_t> { };
template <> struct MinSizeType<64, true> : tag<std::int64_t> { };

template <typename T>
inline auto mod(T x, T y) {
	using widen = typename NextSize<T>::type;
	constexpr std::size_t shiftCnt = 8 * sizeof(T);
	return (static_cast<widen>(x) * static_cast<widen>(y)) >> shiftCnt;
}

template <std::size_t bits>
inline typename MinSizeType<bits, false>::type rand(void);

template <> inline std::uint16_t rand<16>(void) {
	static std::uint16_t state = 1;
    state ^= state << 7;
    state ^= state >> 9;
    state ^= state << 8;
    return state;
}

template <> inline std::uint32_t rand<32>(void) {
	static std::uint32_t state = 2463534242;
    state ^= state << 13;
    state ^= state >> 17;
    state ^= state << 5;
    return state;
}

template <> inline std::uint64_t rand<64>(void) {
	static std::uint64_t state = 8817264546332525;
    state ^= state << 13;
    state ^= state >> 7;
    state ^= state << 17;
    return state;
}

#pragma FUNC_ALWAYS_INLINE
inline const taskBase schedulerBase::lottery(void) {
	const std::size_t sz = schedulerBase::taskList.size();
	static std::vector<std::size_t> intervals;
	intervals.reserve(sz);

	if (schedulerBase::numSleeping != sz - 1) {
		if (intervals.size() != sz) {
			intervals.clear();

			std::size_t bound = 0;
			intervals.push_back(bound);
			for (std::size_t i = 1; i < schedulerBase::taskList.size(); ++i) {
				const taskBase &t = schedulerBase::taskList[i];
				if (!t.isSleeping()) bound += t.priority;
				intervals.push_back(bound);
			}
		}

		const std::size_t ticketSum = intervals.back();
		const auto randVal = rand<16>();
		auto draw = mod(randVal, ticketSum);

		const std::vector<std::size_t>::iterator pos = std::upper_bound(intervals.begin(), intervals.end(), draw);
		const auto idx = pos - intervals.begin();
		schedulerBase::currProc = idx;
		return schedulerBase::taskList[idx];
	}

	return schedulerBase::taskList[0];
}

#pragma vector = WDT_VECTOR
__attribute__((naked, interrupt)) void schedulerBase::preempt(void) {
	taskBase currTask = schedulerBase::taskList[schedulerBase::currProc];

	#if defined (__USEMSP430X__)
		std::uint32_t sp = _get_SP_register();
		std::uint32_t srLoc = reinterpret_cast<std::uint32_t>(currTask.trapframe + 12);
	#else
		std::uint16_t sp = _get_SP_register();
		std::uint16_t srLoc = reinterpret_cast<std::uint16_t>(currTask.trapframe + 12);
	#endif

	std::uint8_t *userStackTop = reinterpret_cast<std::uint8_t *>(sp);
	std::uint8_t *srAddress = reinterpret_cast<std::uint8_t *>(srLoc);

	srAddress[0] = userStackTop[0];
	srAddress[1] = userStackTop[1];
	srAddress[2] = userStackTop[2];
	srAddress[3] = userStackTop[3];
	currTask.trapframe[11] = sp + 4;

	_set_SP_register(srLoc);

	if (currTask.isSleeping()) {
		_enable_interrupts();

		#if defined (__USEMSP430X__)
			asm("   pushx.b r2\n"); // probably wrong
		#else
			asm("   push r2\n");
		#endif
	}

	sys::saveContext();

	_set_SP_register(schedulerBase::sysSp);

	if (currTask.isComplete()) {
		const std::vector<taskBase>::iterator taskToBeErased = schedulerBase::taskList.begin() + schedulerBase::currProc;
		schedulerBase::taskList.erase(taskToBeErased);
		if (++schedulerBase::currProc) schedulerBase::currProc = 0;
		schedulerBase::numSleeping--;
	}

	if (schedulerBase::numSleeping > 0) {

	}

	const taskBase &nextTask = schedulerBase::method();
	#if defined(__USEMSP430X__)
		_set_SP_register(reinterpret_cast<std::uint32_t>(nextTask.trapframe));
	#else
		_set_SP_register(reinterpret_cast<std::uint16_t>(nextTask.trapframe));
	#endif

	sys::restoreContext();
}

#endif
