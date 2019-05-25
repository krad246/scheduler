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

std::uint32_t lfsr113_Bits (void)
{
   static std::uint32_t z1 = 12345, z2 = 12345, z3 = 12345, z4 = 12345;
   unsigned int b;
   b  = ((z1 << 6) ^ z1) >> 13;
   z1 = ((z1 & 4294967294U) << 18) ^ b;
   b  = ((z2 << 2) ^ z2) >> 27;
   z2 = ((z2 & 4294967288U) << 2) ^ b;
   b  = ((z3 << 13) ^ z3) >> 21;
   z3 = ((z3 & 4294967280U) << 7) ^ b;
   b  = ((z4 << 3) ^ z4) >> 12;
   z4 = ((z4 & 4294967168U) << 13) ^ b;
   return (z1 ^ z2 ^ z3 ^ z4);
}

const taskBase schedulerBase::lottery(void) {
	const std::size_t sz = schedulerBase::taskList.size();
	if (schedulerBase::numSleeping != sz - 1) {
		std::size_t ticketSum = 0;
		for (std::size_t i = 1; i < schedulerBase::taskList.size(); ++i) {
			const taskBase &t = schedulerBase::taskList[i];
			ticketSum += t.priority;
		}

		std::uint32_t draw = lfsr113_Bits() % ticketSum;

		std::size_t lb = 0;
		std::size_t hb = 0;
		for (std::size_t i = 1; i < schedulerBase::taskList.size(); ++i) {
			const taskBase &t = schedulerBase::taskList[i];
			if (t.isSleeping()) continue;
			else {
				hb += t.priority;
				if (lb <= draw && draw < hb) return t;
				lb += t.priority;
			}
		}
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
		// mark as dead and let the gc deallocate after a while
		std::vector<taskBase>::iterator taskToBeErased = schedulerBase::taskList.begin() + schedulerBase::currProc;
		schedulerBase::taskList.erase(taskToBeErased);
		if (++schedulerBase::currProc) schedulerBase::currProc = 0;
		schedulerBase::numSleeping--;
	}

//	if (schedulerBase::numSleeping > 0) {
//
//	}

	const taskBase &nextTask = schedulerBase::method();
	#if defined(__USEMSP430X__)
		_set_SP_register(reinterpret_cast<std::uint32_t>(nextTask.trapframe));
	#else
		_set_SP_register(reinterpret_cast<std::uint16_t>(nextTask.trapframe));
	#endif

	sys::restoreContext();
}

#endif
