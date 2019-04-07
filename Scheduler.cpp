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

/**
 * Round robin scheduling method; loops through task list linearly.
 */

Task *Scheduler::roundRobin(Scheduler *arg) {

	/**
	 * Grab the scheduler instance.
	 */

	const Scheduler& sched = *arg;

	/**
	 * Maintain an iterator that continually loops through.
	 */
	static ListIterator<Task *> toRun = sched.queue.begin();

	/**
	 * Grab the task that the iterator points to.
	 */

	Task *runnable = *toRun;

	/**
	 * Grab the return address held in the kernel stack. If it is the idle hook,
	 * increment the iterator to skip it.
	 */

	const volatile std::uint16_t node = (std::uint16_t) runnable;
	const volatile std::uint16_t retAddress = runnable->KernelStackPointer[15];
	if (retAddress == (std::uint16_t) Task::idle) {
		toRun++;
	}

	/**
	 * Grab this new task. Scan ahead and find the first task that is not asleep.
	 */

	runnable = *toRun;
	while (runnable->sleeping) {
		toRun++;
		runnable = *toRun;
	}

	/**
	 * Move the iterator forward for next time.
	 */

	toRun++;

	return runnable;
}

/**
 * Scheduler constructor; accepts a queue of tasks and a scheduling method to run.
 */

Scheduler::Scheduler(TaskQueue& tasks, SchedulingMethod method) : queue(tasks) {

	/**
	 * Grab the list of tasks and inject the idle hook into it. The system must always be running
	 * something.
	 */

	queue = tasks;
	queue.addTask(Task::idle);

	/**
	 * Set the scheduling method.
	 */

	callback = method;

	/**
	 * Singleton pattern. If this is the first instance of the scheduler being built, set the singleton
	 * to this instance. Otherwise, hang / crash.
	 */

	if (sched == nullptr) sched = this;
	else _low_power_mode_4();
}

/**
 * Start the scheduler by configuring the clock modules and keeping track of the system stack
 * state so it can be used later.
 */

void Scheduler::start(std::size_t frequency) {
	SchedStackPointer = (std::uint16_t *) _get_SP_register();
	SystemClock::StartSystemClock(frequency);
}

/**
 * Scheduler tick that performs the context switch. Interrupt switches system to kernel mode,
 * updates system time, cleans up any finished tasks, figures out what to run next, then exits
 * kernel mode by loading the state of the runnable.
 */

#pragma vector = WDT_VECTOR
interrupt void Scheduler::preempt(void) {

	/**
	 * Switch modes.
	 */

	enterKernelMode();

	/**
	 * Do some cleanup.
	 */

	SystemClock::UpdateSystemTime();
	freeCompletedTasks();

	/**
	 * Prep for the next function.
	 */

	const Task *runnable = sched->callback(sched);

	/**
	 * Jump to the next function.
	 */

	exitKernelMode(runnable);
}
