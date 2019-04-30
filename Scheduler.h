/*
 * Scheduler.h
 *
 *  Created on: Apr 5, 2019
 *      Author: krad2
 */

#ifndef SCHEDULER_H_
#define SCHEDULER_H_

#include <msp430.h>
#include <type_traits>
#include <Math.h>
#include <SystemClock.h>
#include <Task.h>

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

	using SchedulingMethod = void (*)(void);

	/**
	 * Scheduling methods supported:
	 * - Round Robin - Loop through each task sequentially.
	 * - Lottery - Probabilistically pick a task based on the priority weight it has.
	 */

	static void lottery(void);
	static void roundRobin(void);

	/**
	 * Scheduler constructor; takes a pointer to a task queue so it can operate on it (add / remove tasks)
	 * and a scheduling method to pick the next task to run.
	 */

	Scheduler(TaskQueue& tasks, SchedulingMethod method = roundRobin);

	/**
	 * Starts the scheduler, which starts the system clock and logs the top of the system stack.
	 * This 2nd step is necessary because kernel code should only use the system stack.
	 */

	void start(std::size_t frequency = (std::size_t) 1000000);

	/**
	 * Sleep function that tasks can call to be suspended for some specified amount of time.
	 */

	static void sleep(std::size_t millis);

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
	 * Most recently picked task. Used to figure out cleanup / sleep state.
	 */

	static ListIterator<Task *> currProc;

	/**
	 * Instance variables for the scheduler so that it can change on the fly.
	 * Scheduler takes a pointer to a task queue so that it can be replaced / modified and
	 * a callback function to determine the next function to run.
	 */

	TaskQueue &queue;
	SchedulingMethod schedule;

	/**
	 * System state including number of tasks sleeping and, if the lottery scheduler is enabled, ticket count.
	 */

	volatile std::size_t numSleeping = 0;
	volatile std::size_t tickets = 0;

	/**
	 * Functions that have to run every time a context switch has to happen.
	 */

	/**
	 * Saves register state of the system at the point of the interrupt.
	 */

	static void saveContext(void);

	/**
	 * Restores register state from a trapframe located in the kernel stack of a program.
	 */

	static void restoreContext(void);

	/**
	 * Completes a task switch by loading the program counter with the address of the new task.
	 */

	static void jumpToNextTask(void);

	/**
	 * Wrapper functions that do a combination of all of the tasks above.
	 */

	static void enterKernelMode(void);
	static void exitKernelMode(void);

	/**
	 * Deletes tasks that have completed.
	 */

	static void freeCompletedTasks(void);

	/**
	 * Wakes sleeping tasks.
	 */

	static void wakeSleepingTasks(void);

	/**
	 * Scheduler tick / preemption function.
	 */

	static interrupt void preempt(void);
};

/**
 * Sleep for the specified duration in milliseconds.
 */

#pragma FUNC_ALWAYS_INLINE
inline void Scheduler::sleep(std::size_t millis) {

	/**
	 * Disable interrupts to avoid race conditions.
	 */

	_disable_interrupts();

	/**
	 * Update time stamp, then call the scheduler tick.
	 */

	(*Scheduler::currProc)->timeStamp = SystemClock::millis;
	(*Scheduler::currProc)->duration = millis;
	(*Scheduler::currProc)->sleeping = true;

	Scheduler::sched->numSleeping++;
	preempt();
}

#endif /* SCHEDULER_H_ */
