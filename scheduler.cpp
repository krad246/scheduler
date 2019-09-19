/*
 * scheduler.cpp
 *
 *  Created on: Sep 11, 2019
 *      Author: krad2
 */

#ifndef SCHEDULER_CPP_
#define SCHEDULER_CPP_

#include <config.h>
#include <task.h>
#include <scheduler_base.h>
#include <scheduler.h>

/**
 * Constructs a scheduler from the specialization implemented in the level above in the hierarchy
 */

template <scheduling_algorithms alg>
scheduler<alg>::scheduler() {
	base_scheduler<alg>::base_scheduler(); // Call super constructor
}

/**
 * Constructs a scheduler given a task list, but also initializes the stack pointer for the OS after
 */

template <scheduling_algorithms alg>
scheduler<alg>::scheduler(const std::initializer_list<task> &task_list) : base_scheduler<alg>(task_list) {
	this->kstack_ptr = _get_SP_register(); // Initializes stack pointer
}

/**
 * Adds a process to the set of processes. Also implemented in the upper level
 */

template <scheduling_algorithms alg>
void scheduler<alg>::add_task(const task &t) {
	base_scheduler<alg>::add_task(t); // Call super.add()
}

/**
 * Removes a process from the set of processes
 */

template <scheduling_algorithms alg>
void scheduler<alg>::cleanup(task &t) {
	_disable_interrupt(); // Enter critical section

	base_scheduler<alg>::cleanup(t); // Call super.cleanup()
	this->leave_kernel_mode(); // Leave critical section
}

extern void driver_init(void);	// Driver initialization function provided by user
extern struct task_config task_cfgs[num_tasks_declared];	// List of tasks and associated configurations as provided by user

template <scheduling_algorithms alg>
void scheduler<alg>::init(void) {
	_disable_interrupt();	// Enter critical section

	driver_init();	// Initialize the hardware

	/**
	 * Iterate through the task list and add it to the underlying process container
	 */

	struct task_config *end_pt = task_cfgs + sizeof(task_cfgs) / sizeof(struct task_config);
	for (struct task_config *it = task_cfgs; it < end_pt; ++it) {
		this->add_task(std::move(task(it->func, it->stack_size, it->priority)));
	}
}

/**
 *	Starts the OS assuming that it has been initialized
 */

template <scheduling_algorithms alg>
void scheduler<alg>::start(void) {
//	if (this->tasks.size() == 0) {
//		// halt
//	}

	// Perform any additional initializations, if needed, in the superclass
	base_scheduler<alg>::start();

	// Configure the scheduler timer interrupt
	WDTCTL = WDT_ADLY_16;
	SFRIE1 |= WDTIE;

	// Schedule and load the first task
	task &first = this->schedule();
	first.load();
}

/**
 * Initializes the OS and configures it given a list of tasks
 */

template <scheduling_algorithms alg>
void scheduler<alg>::start(const std::initializer_list<task> &task_list) {
//	if (task_list.size() == 0) {
//		// halt
//	}

	// Construct the OS with the task list, then start it
	new (this) scheduler(task_list);
	this->start();
}

/**
 * Performs a context switch
 */

template <scheduling_algorithms alg>
inline void scheduler<alg>::context_switch(void) {
	__disable_interrupt();	// Enter critical section
	this->save_context();	// Save current task context
	this->enter_kstack();	// Switch to the OS stack
	task &runnable = this->schedule();	// Determine the next process to run
	this->restore_context(runnable);	// Select that process and load it
}

/**
 * Saves task context for the current process
 */

template <scheduling_algorithms alg>
inline void scheduler<alg>::save_context(void) {
	this->get_current_process().pause();
}

/**
 * Restores task context given a task in order to load it
 */

template <scheduling_algorithms alg>
inline void scheduler<alg>::restore_context(task &runnable) {
	runnable.load();
}

/**
 * Switches to the OS-reserved stack by switching to the top of its stack
 */

template <scheduling_algorithms alg>
inline void scheduler<alg>::enter_kstack(void) {
	_set_SP_register(this->kstack_ptr);
}

/**
 * Returns back to the current process' most recent top of stack
 */

template <scheduling_algorithms alg>
inline void scheduler<alg>::leave_kstack(void) {
	_set_SP_register(this->get_current_process().get_latest_sp());
}

/**
 * Calls the underlying scheduler implementation to determine a task
 */

template <scheduling_algorithms alg>
task &scheduler<alg>::schedule(void) {
	return base_scheduler<alg>::schedule();
}

/**
 * Returns the currently running task
 */

template <scheduling_algorithms alg>
inline task &scheduler<alg>::get_current_process(void) {
	return *this->current_process;
}

/**
 * Returns the state of the currently running thread
 */

template <scheduling_algorithms alg>
const thread_info &scheduler<alg>::get_thread_state(void) {
	return this->get_current_process().get_state();
}


//template <scheduling_algorithms alg>
//inline void scheduler<alg>::enter_kernel_mode(void) {
//	__disable_interrupt();
//	this->get_current_process().pause();
//	this->enter_kstack();
//}

//template <scheduling_algorithms alg>
//inline void scheduler<alg>::leave_kernel_mode(void) {
//	this->schedule().load();
//}

/**
 * Puts a task to sleep on a timer and performs stack manipulation to correctly transfer control to the scheduler
 */

template <scheduling_algorithms alg>
__attribute__((noinline)) void scheduler<alg>::sleep(const std::size_t ticks) {
	__disable_interrupt();	// Enter critical section

	// Set the sleep counter up for the calling process
	this->current_process->sleep(ticks);

	/**
	 * Upon entry into sleep(), the address of the next normal instruction was pushed
	 * Grab this instruction and reformat by swapping the upper and lower words for a BRA instruction
	 */

	const register std::uint16_t temp = *reinterpret_cast<std::uint16_t *>(_get_SP_register());
	*reinterpret_cast<std::uint16_t *>(_get_SP_register()) = *reinterpret_cast<std::uint16_t *>(_get_SP_register() + 2) << 12;
	*reinterpret_cast<std::uint16_t *>(_get_SP_register() + 2) = temp;

	// Call the scheduler
	abstract_scheduler::preempt();
}

template <scheduling_algorithms alg>
__attribute__((noinline)) void scheduler<alg>::block(void) {
	__disable_interrupt();	// Enter critical section

	// Set the blocking flag on the current process
	this->current_process->block();

	/**
	 * Upon entry into sleep(), the address of the next normal instruction was pushed
	 * Grab this instruction and reformat by swapping the upper and lower words for a BRA instruction
	 */

	const register std::uint16_t temp = *reinterpret_cast<std::uint16_t *>(_get_SP_register());
	*reinterpret_cast<std::uint16_t *>(_get_SP_register()) = *reinterpret_cast<std::uint16_t *>(_get_SP_register() + 2) << 12;
	*reinterpret_cast<std::uint16_t *>(_get_SP_register() + 2) = temp;

	// Call the scheduler
	abstract_scheduler::preempt();
}

/**
 * Atomically unblocks a process when requested
 */

template <scheduling_algorithms alg>
void scheduler<alg>::unblock(void) {
	_disable_interrupt();	// Enter critical section
	this->current_process->unblock();
	_enable_interrupt();	// Exits critical section
}

#endif
