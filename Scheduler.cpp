/*
 * Scheduler.cpp
 *
 *  Created on: Apr 5, 2019
 *      Author: krad2
 */

#include <Scheduler.h>

Scheduler *Scheduler::sched = nullptr;
std::uint16_t *Scheduler::SchedStackPointer = nullptr;
ListIterator<Task *> Scheduler::currProc = Scheduler::sched->queue.begin();

void Scheduler::lottery(void) {

	/**
	 * Maintain an iterator that loops through and a count of how many tasks to go through.
	 */

	register ListIterator<Task *> task = Scheduler::sched->queue.begin();
	const std::size_t sz = Scheduler::sched->queue.size();

	/**
	 * Retrieve the 'pool of tickets' from the queue of tasks. Drop sleeping tasks from the pool if there are any.
	 */

	const std::size_t numSleeping = Scheduler::sched->numSleeping;
	std::size_t pool = Scheduler::sched->tickets;
	if (numSleeping > 0) {
		for (std::size_t i = 0; i < sz - 1; ++i) {
			if ((*task)->sleeping) pool -= (*task)->priority;
			task++;
		}
	}

	/**
	 * If all tasks but the idle() hook are asleep, just assign that and leave.
	 */

	if (pool == 1) {
		task = Scheduler::sched->queue.end();
	} else {

		/**
		 * Compute a random number for the lottery 'draw'.
		 */

		const std::size_t draw = rand<16>();

		/**
		 *  Map that into the range of tickets via modulo(). This is an approximation to the modulo function.
		 */

		const std::size_t map = multiply(draw, pool) >> (sizeof(std::size_t) << 3);
//		const auto dmp = divmod(draw, pool);
//		const std::size_t map = dmp.remainder;

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

		task = Scheduler::sched->queue.begin();
		for (std::size_t i = 0; i < sz - 1; ++i) {

			/**
			 * Sleeping tasks are skipped entirely. This is for the cases where not all of the possible tasks are sleeping.
			 */

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
	}

	/**
	 * Update currProc with the task chosen and leave.
	 */

	Scheduler::currProc = task;
}

/**
 * Round robin scheduling method; loops through task list linearly.
 */

void Scheduler::roundRobin(void) {

	/**
	 * Maintain an iterator that continually loops through.
	 */

	register ListIterator<Task *> task = Scheduler::currProc;

	/**
	 * For weighted round robin scheduling, there needs to be a notion of the number of times
	 * a task has been allowed to run. Only once the task has been run for its weight of time are
	 * we allowed to move to the next task.
	 */

	static std::size_t numTimesRun = 0;

	/**
	 * Check if the current task is an idle(). If it is the only runnable (all others are sleeping) then run it.
	 * Otherwise skip it.
	 */

	if ((*task)->isIdle()) {

		/**
		 * Regardless of whether or not we run idle(), reset the counter because we need to start fresh on a task.
		 */

		numTimesRun = 0;

		/**
		 * If N - 1 tasks (the other one is the idle hook) are asleep, then the idle hook is all we can run.
		 * Otherwise, skip the idle hook.
		 */

		const std::size_t sz = Scheduler::sched->queue.size();
		const std::size_t numSleeping = Scheduler::sched->numSleeping;
		if (numSleeping < sz - 1) task++;
		else {

			/**
			 * Leave with idle().
			 */

			numTimesRun++;
			Scheduler::currProc = task;
			return;
		}
	}

	/**
	 * If the current task has run enough times, reset the counter. In the event this happens, find a new task to run.
	 * Check for the first task that is not sleeping and also is not the idle hook.
	 */

	const bool TaskRunEnoughTimes = (numTimesRun >= (*task)->priority);
	if (TaskRunEnoughTimes) {

		/**
		 * Reset the counter and move the task iterator forward so the same thing doesn't get rerun.
		 */

		numTimesRun = 0;
		task++;

		/**
		 * Need to compare the return address of each task to the idle hook; if it isn't the idle hook and it isn't
		 * sleeping, then this is a valid task to run.
		 */

		/**
		 * Find the first task in the list that ISN'T sleeping.
		 */

		while ((*task)->sleeping) task++;
	}

	/**
	 * Leave with this task, incrementing its run counter. Update currProc with the task chosen.
	 */

	numTimesRun++;
	Scheduler::currProc = task;
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
	 * If the lottery scheduler is enabled, then total up the tickets.
	 */

	ListIterator<Task *> task = queue.begin();
	for (std::size_t i = 0; i < queue.size(); ++i) {
		tickets += (*task)->priority;
		task++;
	}

	/**
	 * Set the scheduling method.
	 */

	schedule = method;

	/**
	 * Singleton pattern. If this is the first instance of the scheduler being built, set the singleton
	 * to this instance. Otherwise, hang / crash.
	 */

	if (sched == nullptr) sched = this;
	else _low_power_mode_4();
}

/**
 * Start the scheduler by configuring the clock modules and keeping track of the system stack
 * state so it can be used later, as well as actually queueing up a task.
 */

void Scheduler::start(std::size_t frequency) {
	Scheduler::currProc = queue.begin();
	Scheduler::SchedStackPointer = (std::uint16_t *) _get_SP_register();
	SystemClock::StartSystemClock(frequency);

	/**
	 * Ignore system tick interrupt for the first boot.
	 */

	_disable_interrupts();

	/**
	 * Queue up a function.
	 */

	Scheduler::sched->schedule();

	/**
	 * Jump to the next function.
	 */

	Scheduler::exitKernelMode();
}

/**
 * Saves system state from before the context switch.
 */

#pragma FUNC_ALWAYS_INLINE
inline void Scheduler::saveContext(void) {
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
}

/**
 * Loads system state / context of new task.
 */

#pragma FUNC_ALWAYS_INLINE
inline void Scheduler::restoreContext(void) {
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
}

/**
 * Switches stacks from user to kernel stacks for subsequent code. Copies the pushed words on the user stack
 * to the kernel stack.
 */

#pragma FUNC_ALWAYS_INLINE
inline void Scheduler::switchStacks(void) {
	std::uint16_t *UserStackPointer = (std::uint16_t *) _get_SP_register();
	(*Scheduler::currProc)->KernelStackPointer[12] = *UserStackPointer;
	UserStackPointer++;
	(*Scheduler::currProc)->KernelStackPointer[13] = *UserStackPointer;
	UserStackPointer++;

	asm volatile("   add.w #4, sp\n" \
				 "   mov.w sp, r4\n");

	(*Scheduler::currProc)->KernelStackPointer[11] = (std::uint16_t) UserStackPointer;

	_set_SP_register((std::uint16_t) (12 + (*Scheduler::currProc)->KernelStackPointer));
}

/**
 * Loads the program counter and status register with the SR & address of the new task.
 */

#pragma FUNC_ALWAYS_INLINE
inline void Scheduler::jumpToNextTask(void) {
	asm volatile ( \
		" 	pop r2\n" \
		" 	mov.w @sp, r5\n" \
		"   mov.w r4, sp\n" \
		"  	br r5\n" \
	);
}

/**
 * Setup to switch stacks on entry into a new task.
 */

#pragma FUNC_ALWAYS_INLINE
inline void Scheduler::enterTaskSetup(void) {
	_set_SP_register((std::uint16_t) (*Scheduler::currProc)->UserStackPointer);
}

/**
 * When entering the scheduler tick / kernel code, must save the context and switch to the
 * system stack for any computation needed.
 */

#pragma FUNC_ALWAYS_INLINE
inline void Scheduler::enterKernelMode(void) {
	Scheduler::switchStacks();

	if ((*Scheduler::currProc)->sleeping) {
		_enable_interrupt();
		asm volatile(" 	push r2\n");
	}

	Scheduler::saveContext();
	_set_SP_register((std::uint16_t) Scheduler::SchedStackPointer);
}

/**
 * When exiting the scheduler tick / kernel code, must restore the context of the new function by
 * loading the stack pointer of the new task, popping off the register state, and jumping to the
 * function.
 */

#pragma FUNC_ALWAYS_INLINE
inline void Scheduler::exitKernelMode(void) {
	_set_SP_register((std::uint16_t) (*Scheduler::currProc)->KernelStackPointer);
	restoreContext();
	jumpToNextTask();
}

/**
 * Cleanup function that removes tasks that have returned. Figure out which process has returned, move the
 * process pointer forward so it doesn't become invalid, then pop the task to be removed.
 */

#pragma FUNC_ALWAYS_INLINE
inline void Scheduler::freeCompletedTasks(void) {

	/**
	 * If the task we just switched out of is done, we need to clean it up.
	 */

	if ((*Scheduler::currProc)->complete()) {

		/**
		 * Grab the task to be freed and move the process pointer so it doesn't become invalid.
		 */

		ListIterator<Task *> TaskToBeFreed = Scheduler::currProc;
		Scheduler::currProc++;

		/**
		 * Drop the task to be freed and remove it from the ticket and sleep pools.
		 */

		Scheduler::sched->tickets -= (*TaskToBeFreed)->priority;
		Scheduler::sched->queue.pop(TaskToBeFreed);
		Scheduler::sched->numSleeping--;
	}
}

#pragma FUNC_ALWAYS_INLINE
inline void Scheduler::wakeSleepingTasks(void) {

	/**
	 * Wakes sleeping tasks if there are any to wake up.
	 */

	if (Scheduler::sched->numSleeping > 0) {

		/**
		 * Loop over each task and check if it's sleeping. If it has slept long enough then 'unsleep' it.
		 */

		register ListIterator<Task *> TaskIterator = Scheduler::sched->queue.begin();
		const std::size_t sz = Scheduler::sched->queue.size();

		for (std::size_t i = 0; i < sz; i++) {

			/**
			 * Check if task has slept long enough.
			 */

			const volatile bool timeUp = SystemClock::millis >= (*TaskIterator)->timeStamp + (*TaskIterator)->duration;

			/**
			 * Check sleep status.
			 */

			if ((*TaskIterator)->sleeping && timeUp) {

				/**
				 * If it has slept long enough then remove it from the sleep queue.
				 */

				(*TaskIterator)->sleeping = false;
				Scheduler::sched->numSleeping--;
			}

			/**
			 * Move to the next task.
			 */

			TaskIterator++;
		}
	}
}

/**
 * Scheduler tick that performs the context switch. Interrupt switches system to kernel mode,
 * updates system time, cleans up any finished tasks, figures out what to run next, then exits
 * kernel mode by loading the runnable.
 */

#pragma vector = WDT_VECTOR
__attribute__((naked, interrupt)) void Scheduler::preempt(void) {

	/**
	 * Switch modes.
	 */

	Scheduler::enterKernelMode();

	/**
	 * Do some cleanup.
	 */

	SystemClock::UpdateSystemTime();
	Scheduler::freeCompletedTasks();
	Scheduler::wakeSleepingTasks();

	/**
	 * Prep for the next function.
	 */

	Scheduler::sched->schedule();

	/**
	 * Jump to the next function.
	 */

	Scheduler::exitKernelMode();
}
