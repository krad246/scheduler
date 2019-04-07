/*
 * Scheduler.h
 *
 *  Created on: Apr 5, 2019
 *      Author: krad2
 */

#ifndef SCHEDULER_H_
#define SCHEDULER_H_

#include <msp430.h>
#include <Math.h>
#include <SystemClock.h>
#include "Task.h"

/**
 * Forward declarations of other classes that work with the Scheduler class.
 */

class TaskQueue;
class SystemClock;

/**
 * Scheduler class:
 * Preemptive scheduler class.
 */

class Scheduler {
public:

	/**
	 * Function typedef for functions that return tasks based on some scheduling algorithm.
	 */

	using SchedulingMethod = Task *(*)(Scheduler *);

	/**
	 * Scheduling methods supported:
	 * - Round Robin - Loop through each task sequentially.
	 * - Lottery - Probabilistically pick a task based on the priority weight it has.
	 */

	static Task *lottery(Scheduler *arg);
	static Task *roundRobin(Scheduler *arg);

	/**
	 * Scheduler constructor; takes a pointer to a task queue so it can operate on it (add / remove tasks)
	 * and a scheduling method to pick the next task to run.
	 */

	Scheduler(TaskQueue& tasks, SchedulingMethod method = Scheduler::roundRobin);

	/**
	 * Starts the scheduler, which starts the system clock and logs the top of the system stack.
	 * This 2nd step is necessary because kernel code should only use the system stack.
	 */

	void start(std::size_t frequency = (std::size_t) 1000000);

	/**
	 * Sleep function that tasks can call to be suspended for some specified amount of time.
	 */

	static void sleep(std::size_t micros);

private:
	/**
	 * Singleton for the scheduler. Only one scheduler can run at any point in time!
	 */

	static Scheduler *sched;

	/**
	 * System stack pointer for the running instance of the scheduler.
	 */

	static std::uint16_t *SchedStackPointer;

	/**
	 * Instance variables for the scheduler so that it can change on the fly.
	 * Scheduler takes a pointer to a task queue so that it can be replaced / modified and
	 * a callback function to determine the next function to run.
	 */

	TaskQueue &queue;
	SchedulingMethod callback;

	/**
	 * Fight the compiler calling convention in the scheduler tick by removing unnecessarily pushed registers.
	 */
#pragma FUNC_ALWAYS_INLINE
	static inline void popAutomaticallyPushedRegisters(void) {
		asm volatile ( \
			"   pop r9 \n" \
			"	pop r10 \n" \
			"   pop r11 \n" \
			"   pop r12 \n" \
			"	pop r13 \n" \
			"	pop r14 \n" \
			"	pop r15 \n" \
		);
	}

	/**
	 * Saves system state from before the context switch.
	 */
#pragma FUNC_ALWAYS_INLINE
	static inline void saveContext(void) {
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
	static inline void restoreContext(void) {
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
	 * Loads the program counter and status register with the SR & address of the new task.
	 */
#pragma FUNC_ALWAYS_INLINE
	static inline void jumpToNextTask(void) {
		asm volatile (
			" 	reti \n"
		);
	}


	/**
	 * When entering the scheduler tick / kernel code, must save the context and switch to the
	 * system stack for any computation needed.
	 */
#pragma FUNC_ALWAYS_INLINE
	static inline void enterKernelMode(void) {
		popAutomaticallyPushedRegisters();
		saveContext();
		_set_SP_register((std::uint16_t) SchedStackPointer);
	}

	/**
	 * When exiting the scheduler tick / kernel code, must restore the context of the new function by
	 * loading the stack pointer of the new task, popping off the register state, and jumping to the
	 * function.
	 */
#pragma FUNC_ALWAYS_INLINE
	static inline void exitKernelMode(const Task *runnable) {
		_set_SP_register((std::uint16_t) runnable->KernelStackPointer);
		restoreContext();
		jumpToNextTask();
	}

	/**
	 * Cleanup function that removes tasks that have returned.
	 */
#pragma FUNC_ALWAYS_INLINE
	static inline void freeCompletedTasks(void) {

		/**
		 * Get an iterator to loop over the task queue.
		 */

		register ListIterator<Task *> TaskIterator = sched->queue.begin();

		/**
		 * Loop over each of the tasks in the queue. For each task, check if it has completed;
		 * if it has returned twice, then the status register will overwrite the function
		 * return address. Maintain a pointer to the task that has completed, advance the iterator
		 * so it doesn't become invalid, then delete the completed task.
		 */

		for (std::size_t i = 0; i < Scheduler::sched->queue.size(); ++i) {

			/**
			 * Check if the task is complete.
			 */

			if ((*TaskIterator)->complete()) {

				/**
				 * Maintain a pointer to the completed task; it will be freed.
				 */

				ListIterator<Task *> TaskToBeFreed = TaskIterator;

				/**
				 * Move forward before deleting this task from the queue so that the underlying
				 * linked list can maintain connectivity without invalidating the iterator.
				 */

				TaskIterator++;

				/**
				 * Pop the task from the task queue.
				 */

				Scheduler::sched->queue.pop(TaskToBeFreed);
			} else {
//				(*TaskIterator)->KernelStackPointer[15] = (*TaskIterator)->KernelStackPointer[13];

				/**
				 * Otherwise, move on to the next task in the list.
				 */

				TaskIterator++;
			}
		}
	}

	/**
	 * Scheduler tick / preemption function.
	 */

	static interrupt void preempt(void);
};

#endif /* SCHEDULER_H_ */
