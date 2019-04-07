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

	/**
	 * Grab the scheduler instance.
	 */

	const Scheduler& sched = *arg;

	/**
	 * Maintain an iterator that loops through and a count of how many tasks to go through.
	 */

	ListIterator<Task *> task = sched.queue.begin();
	const std::size_t sz = sched.queue.size();

	/**
	 * If idle() is the only runnable function at all, then we just return the idle hook.
	 */

	const std::size_t numSleeping = sched.queue.numSleeping;
	if (numSleeping == sz - 1) {
		return *task;
	}

	/**
	 * Loop through the interval [0, pool) and see if the draw is in an interval
	 * [X1, X2) where X2 - X1 = task.tickets.
	 */

	/**
	 * Retrieve the 'pool of tickets' from the queue of tasks. We drop idle() by default.
	 */

	const std::size_t pool = sched.queue.tickets - 1;

	/**
	 * Compute a random number for the lottery 'draw'.
	 */

	const std::size_t draw = rand<16>();

	/**
	 *  Map that into the range of tickets via modulo(). This is an approximation to the modulo function.
	 */

	const std::size_t map = multiply(draw, pool) >> (sizeof(std::size_t) << 3);

	/**
	 * Maintain the interval boundaries.
	 */

	std::size_t lowerTicketBound = 0;
	std::size_t higherTicketBound = 0;

	/**
	 * Loop through the 'pool' of tickets by using the interval variables to create a 'slice'
	 * corresponding to a task. If the random draw is within range, then this is the task that
	 * will be picked.
	 */

	for (std::size_t i = 0; i < sz; ++i) {

		/**
		 * If we see idle(), we skip it without counting its tickets at all.
		 */

		const std::uint16_t retAddress = (*task)->KernelStackPointer[15];
		if (retAddress == (std::uint16_t) Task::idle) {
			task++;
		}

		if ((*task)->sleeping) {
			task++;
			continue;
		}

		/**
		 * Otherwise, we iterate normally and perform the bounds checks as usual. When we are in
		 * the right interval, we return the task that spans that cut.
		 */

		higherTicketBound += (*task)->priority;
		if (lowerTicketBound <= map && map < higherTicketBound) break;
		lowerTicketBound += (*task)->priority;
		task++;
	}

	return *task;
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

	static ListIterator<Task *> task = sched.queue.begin();

	/**
	 * Grab the return address held in the kernel stack. If it is the idle hook,
	 * increment the iterator to skip it unless all other tasks are sleeping.
	 */

	const std::uint16_t retAddress = (*task)->KernelStackPointer[15];
	const std::size_t numSleeping = sched.queue.numSleeping;
	if (retAddress == (std::uint16_t) Task::idle) {
		if (numSleeping != sched.queue.size() - 1) {
			task++;
		} else return *task;
	}

	/**
	 * Scan ahead and find the first task that is not asleep.
	 */

	while ((*task)->sleeping) {
		task++;
	}

	/**
	 * Return the task that isn't asleep.
	 */

	ListIterator<Task *> runnable = task;

	/**
	 * Move the iterator forward for next time.
	 */

	task++;

	return *runnable;
}

/**
 * Scheduler constructor; accepts a queue of tasks and a scheduling method to run.
 */

Scheduler::Scheduler(TaskQueue& tasks, SchedulingMethod method) : queue(tasks) {

	/**
	 * Grab the list of tasks and inject the idle hook into it if it is empty. The system must always be running
	 * something.
	 */

	queue = tasks;
	if (tasks.size() == 0) queue.addTask(Task::idle);

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
