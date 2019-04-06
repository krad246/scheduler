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

	const std::size_t count = sched.queue.size();
	static ListIterator<Task *> toRun = sched.queue.begin();

	Task *runnable = *toRun;
	toRun++;
	if ((*toRun)->KernelStackPointer[15] == (std::uint16_t) Task::idle) {
		toRun++;
	}

	if (toRun.pos() >= count) {
		toRun = sched.queue.begin();
	}

	runnable = *toRun;

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
	Scheduler::enterKernelCode();
	SystemClock::UpdateSystemTime();

	Scheduler::freeCompletedTasks();

	Task *runnable = Scheduler::sched->callback(Scheduler::sched);
	Scheduler::exitKernelCode(runnable);
}
