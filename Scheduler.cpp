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
//	Scheduler::enterKernelCode();

//			popAutomaticallyPushedRegisters();
//			saveContext();
	asm volatile ( \
		"   pop r11 \n" \
		"   pop r12 \n" \
		"	pop r13 \n" \
		"	pop r14 \n" \
		"	pop r15 \n" \
	);

	asm volatile ( \
		"	push r4 \n" \
		"	push r5 \n" \
		"	push r6 \n" \
		"	push r7 \n" \
		"	push r8 \n" \
		"	push r9 \n" \
		"	push r10 \n" \
		"	push r11 \n" \
		"	push r12 \n" \
		"	push r13 \n" \
		"	push r14 \n" \
		"	push r15 \n" \
	);
			_set_SP_register((std::uint16_t) Scheduler::SchedStackPointer);
	SystemClock::UpdateSystemTime();

//	TaskQueue ReplacementTaskQueue = Scheduler::sched->queue;
//	ListIterator<Task *> TaskIterator = Scheduler::sched->queue.begin();
//	for (std::size_t i = 0; i < ReplacementTaskQueue.size(); ++i) {
//		Task *runnable = *TaskIterator;
//		if (runnable->complete()) {
//			ListIterator<Task *> TaskToBeFreed = TaskIterator;
//			ReplacementTaskQueue.pop(TaskToBeFreed);
//		}
//		TaskIterator++;
//	}

//	Scheduler::sched->queue = ReplacementTaskQueue;

	Task *runnable = Scheduler::sched->callback(Scheduler::sched);
//	if (runnable->complete()) {
////		ListIterator<Task *> TaskToBeFreed = Scheduler::sched->queue.find(runnable);
////		Scheduler::sched->queue.pop(TaskToBeFreed);
//
//		runnable->KernelStackPointer[13] = (std::uint16_t) Task::idle;
//		runnable->KernelStackPointer[12] = GIE;
//	}

	if (runnable->KernelStackPointer[13] == GIE) {
	//		ListIterator<Task *> TaskToBeFreed = Scheduler::sched->queue.find(runnable);
	//		Scheduler::sched->queue.pop(TaskToBeFreed);

			runnable->KernelStackPointer[13] = (std::uint16_t) Task::idle;
			runnable->KernelStackPointer[12] = GIE;
		}



	_set_SP_register((std::uint16_t) runnable->KernelStackPointer);
	asm volatile ( \
		"	pop r15 \n" \
		"	pop r14 \n" \
		"	pop r13 \n" \
		"	pop r12 \n" \
		"	pop r11 \n" \
		"	pop r10 \n" \
		"	pop r9 \n" \
		"	pop r8 \n" \
		"	pop r7 \n" \
		"	pop r6 \n" \
		"	pop r5 \n" \
		"	pop r4 \n" \
	);

	asm volatile (
		" 	reti \n"
	);
//	restoreContext();
//	jumpToNextTask();
//	Scheduler::exitKernelCode(runnable);
}
