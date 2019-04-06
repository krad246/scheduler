/*
 * Scheduler.cpp
 *
 *  Created on: Apr 5, 2019
 *      Author: krad2
 */

#include <Scheduler.h>

Scheduler *Scheduler::sched = nullptr;
std::uint16_t *Scheduler::SchedStackPointer = nullptr;

Task *Scheduler::lottery(Scheduler *arg) {
	const Scheduler& sched = *arg;

	const std::uint16_t draw = rand<16>();
	std::size_t ticketSum = 0;

	std::size_t size = sched.queue.size();
	for (std::size_t i = 0;  i < size; ++i) {
		ticketSum += sched.queue[i]->priority;
	}

	std::size_t lowerTicketBound = 0;
	std::size_t higherTicketBound = 0;
	for (std::size_t i = 0; i < size; ++i) {
		lowerTicketBound += sched.queue[i]->priority;

		higherTicketBound += sched.queue[i]->priority;
	}

	return sched.queue.front();
}

Task *Scheduler::roundRobin(Scheduler *arg) {
	const Scheduler& sched = *arg;

	static std::size_t iter = 0;
	Task *runnable = sched.queue[iter];

	if (++iter >= sched.queue.size()) iter = 0;

	return runnable;
}

Scheduler::Scheduler(TaskQueue& tasks, SchedulingMethod method) : queue(tasks) {
	queue = tasks;
	queue.addTask(Task::idle);

	callback = method;

	Scheduler::sched = this;
}

Scheduler::~Scheduler() {

}

void Scheduler::start(std::size_t frequency) {
	SchedStackPointer = (std::uint16_t *) _get_SP_register();
	SystemClock::StartSystemClock(frequency);
}

#pragma vector = WDT_VECTOR
interrupt void Scheduler::preempt(void) {
	Scheduler::enterIsr();
	_set_SP_register((std::uint16_t) Scheduler::SchedStackPointer);

	SystemClock::micros += SystemClock::microsPerTick;
	SystemClock::fractionalMillis += SystemClock::microsPerTick;
	if (SystemClock::fractionalMillis > 1000) {
		SystemClock::fractionalMillis -= 1000;
		SystemClock::millis += 1;
	}

	Task *runnable = Scheduler::sched->callback(Scheduler::sched);
	if (runnable->KernelStackPointer[13] == GIE) {
		runnable->KernelStackPointer[13] = (std::uint16_t) Task::idle;
		runnable->KernelStackPointer[12] = GIE;
	}

	_set_SP_register((std::uint16_t) runnable->KernelStackPointer);
	Scheduler::exitIsr();
}
