/*
 * scheduler.cpp
 *
 *  Created on: May 14, 2019
 *      Author: krad2
 */

#ifdef SCHEDULER_H_

std::size_t scheduler::currProc = 1;
std::vector<task> scheduler::taskList = std::vector<task>();
std::size_t scheduler::numSleeping = 0;
schedulingMethod scheduler::method = nullptr;

#if defined (__USEMSP430X__)
	std::uint32_t scheduler::sysSp = 0;
#else
	std::uint16_t scheduler::sysSp = 0;
#endif

constexpr unsigned gcd(unsigned const a, unsigned const b) {
	return b ? gcd(b, a % b) : a;
}

constexpr unsigned lcm(int const a, int const b) {
	return abs(a * b) / gcd(abs(a), abs(b));
}

scheduler::scheduler(schedulingMethod m, const std::initializer_list<task>& tasks) {
	scheduler::taskList.push_back(task(task::idle));

	if (m == scheduler::stride) {
		auto t0 = tasks.begin();
		auto t1 = tasks.begin() + 1;

		auto p0 = t0->priority;
		auto p1 = t0->priority;

		auto baseStride = lcm(p0, p1);
		for (auto i = begin(tasks) + 2; i < end(tasks); ++i) {
			baseStride = lcm(baseStride, i->priority);
		}

		for (auto i = begin(tasks); i < end(tasks); ++i) {
			task strideTask(*i);
			strideTask.priority = baseStride / i->priority;
			scheduler::taskList.push_back(strideTask);
		}

	} else {
		for (auto i = begin(tasks); i < end(tasks); ++i) {
			scheduler::taskList.push_back(*i);
		}
	}

	scheduler::method = m;
}

void scheduler::start(void) {
	scheduler::sysSp = _get_SP_register();

	const task &nextTask = scheduler::taskList[scheduler::currProc];
	#if defined(__USEMSP430X__)
		_set_SP_register(reinterpret_cast<std::uint32_t>(nextTask.trapframe));
	#else
		set_SP_register(reinterpret_cast<std::uint16_t>(nextTask.trapframe));
	#endif

	sys::restoreContext();
}

#pragma FUNC_ALWAYS_INLINE
inline const task scheduler::roundRobin(void) {
	static volatile std::size_t numTimesRun = 0;
	const std::size_t sz = scheduler::taskList.size();

	if (scheduler::numSleeping != sz - 1) {
		while (1) {
			const task &t = scheduler::taskList[scheduler::currProc];
			if (t.isSleeping()) scheduler::currProc++;
			else {
				const bool taskRunEnoughTimes = (numTimesRun >= t.priority);

				if (taskRunEnoughTimes) {
					numTimesRun = 0;
					scheduler::currProc++;
					if (scheduler::currProc >= sz) {
						scheduler::currProc = (scheduler::numSleeping != sz - 1);
					}
				}

				numTimesRun++;
				return scheduler::taskList[scheduler::currProc];
			}
		}
	}

	return scheduler::taskList[0];
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

#pragma FUNC_ALWAYS_INLINE
template <typename T> inline auto mod(T x, T y) {
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
inline const task scheduler::lottery(void) {
	const std::size_t sz = scheduler::taskList.size();
	static std::vector<std::size_t> intervals;
	intervals.reserve(sz);

	if (scheduler::numSleeping != sz - 1) {
		if (intervals.size() != sz) {
			intervals.clear();

			std::size_t bound = 0;
			intervals.push_back(bound);
			for (std::size_t i = 1; i < scheduler::taskList.size(); ++i) {
				const task &t = scheduler::taskList[i];
				if (!t.isSleeping()) bound += t.priority;
				intervals.push_back(bound);
			}
		}

		const std::size_t ticketSum = intervals.back();
		const auto randVal = rand<16>();
		auto draw = mod(randVal, ticketSum);

		const std::vector<std::size_t>::iterator pos = std::upper_bound(intervals.begin(), intervals.end(), draw);
		const auto idx = pos - intervals.begin();
		scheduler::currProc = idx;
		return scheduler::taskList[idx];

//		std::size_t ticketSum = 0;
//		for (auto i = schedulerBase::taskList.begin() + 1; i < schedulerBase::taskList.end(); ++i) {
//			ticketSum += i->priority;
//		}
//
//		const auto randVal = rand<16>();
//		auto draw = mod(randVal, ticketSum);
//
//		std::size_t lb = 0;
//		std::size_t hb = 0;
//		for (auto i = schedulerBase::taskList.begin() + 1; i < schedulerBase::taskList.end(); ++i) {
//			const taskBase &t = *i;
//			if (t.isSleeping()) continue;
//			else {
//				hb += i->priority;
//				if (lb <= draw && draw < hb) {
//					schedulerBase::currProc = i - schedulerBase::taskList.begin();
//					return t;
//				}
//				lb += i->priority;
//			}
//		}
	}

	return scheduler::taskList[0];
}

#pragma FUNC_ALWAYS_INLINE
inline const task scheduler::stride(void) {
	const std::size_t sz = scheduler::taskList.size();

	if (scheduler::numSleeping != sz - 1) {

	}

	return scheduler::taskList[0];
}

#pragma vector = WDT_VECTOR
__attribute__((naked, interrupt)) void scheduler::preempt(void) {
	task &currTask = scheduler::taskList[scheduler::currProc];

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

	_set_SP_register(scheduler::sysSp);

	if (currTask.isComplete()) {
		const std::vector<task>::iterator taskToBeErased = scheduler::taskList.begin() + scheduler::currProc;
		scheduler::taskList.erase(taskToBeErased);
		if (++scheduler::currProc) scheduler::currProc = 0;
		scheduler::numSleeping--;
	}

	if (scheduler::numSleeping > 0) {

	}

	const task &nextTask = scheduler::method();

	#if defined(__USEMSP430X__)
		_set_SP_register(reinterpret_cast<std::uint32_t>(nextTask.trapframe));
	#else
		_set_SP_register(reinterpret_cast<std::uint16_t>(nextTask.trapframe));
	#endif

	sys::restoreContext();
}

#endif
