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
void scheduler<alg>::cleanup(const task &t) {
	base_scheduler<alg>::cleanup(t); // Call super.cleanup()
}

extern void driver_init(void);								// Driver initialization function provided by user

template <scheduling_algorithms alg>
void scheduler<alg>::init(void) {
	_disable_interrupt();	// Enter critical section

	driver_init();	// Initialize the hardware

	/**
	 * Iterate through the task list and add it to the underlying process container
	 */

	constexpr auto num_cfgs = sizeof(task_cfgs) / sizeof(struct task_config);
	const struct task_config *end_pt = task_cfgs + num_cfgs;
	for (struct task_config *it = const_cast<struct task_config *>(task_cfgs); it < end_pt; ++it) {
		this->add_task(task(it->func, it->stack_size, it->priority));
	}

	_enable_interrupt();
}

/**
 *	Starts the OS assuming that it has been initialized
 */

extern void watchdog_init(void);

template <scheduling_algorithms alg>
void scheduler<alg>::start(void) {
	_disable_interrupt();

	// Perform any additional initializations, if needed, in the superclass
	base_scheduler<alg>::start();

	watchdog_init();

	// Schedule and load the first task
	task &first = this->schedule();
	first.load();
}

/**
 * Initializes the OS and configures it given a list of tasks
 */

template <scheduling_algorithms alg>
void scheduler<alg>::start(const std::initializer_list<task> &task_list) {
	// Construct the OS with the task list, then start it
	new (this) scheduler(task_list);
	this->start();
}

/**
 * Performs a context switch
 */

template <scheduling_algorithms alg>
inline void scheduler<alg>::context_switch(void) {
	_disable_interrupt();	// Enter critical section
	this->save_context();	// Save current task context
	this->enter_kstack();	// Switch to the OS stack
	this->service_interrupts(); // Service interrupts

	if (this->isr_sched_queue.size() > 0) {	// If any interrupts available to be handled, handle them instead
		task &driver_handler = const_cast<task &>(this->isr_sched_queue.top());
		this->current_process = &driver_handler;
		this->restore_context(driver_handler);
	} else {	// Else handle normal processes
		task &runnable = this->schedule();	// Determine the next process to run
		if (this->get_current_process().complete()) {
			this->cleanup(this->get_current_process());
		}

		this->restore_context(runnable);	// Select that process and load it
	}
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

#if defined(__LARGE_CODE_MODEL__) || defined(__LARGE_DATA_MODEL__)
/**
 * Switches to the OS-reserved stack by switching to the top of its stack
 */

template <scheduling_algorithms alg>
inline std::uint32_t scheduler<alg>::enter_kstack(void) {
	register std::uint32_t sp_backup = _get_SP_register();
	_set_SP_register(this->kstack_ptr);
	return sp_backup;
}

/**
 * Returns back to the current process' most recent top of stack
 */

template <scheduling_algorithms alg>
inline void scheduler<alg>::leave_kstack(const std::uint32_t new_sp) {
	_set_SP_register(new_sp);
}
#else
/**
 * Switches to the OS-reserved stack by switching to the top of its stack
 */

template <scheduling_algorithms alg>
inline std::uint16_t scheduler<alg>::enter_kstack(void) {
	register std::uint16_t sp_backup = _get_SP_register();
	_set_SP_register(this->kstack_ptr);
	return sp_backup;
}

/**
 * Returns back to the current process' most recent top of stack
 */

template <scheduling_algorithms alg>
inline void scheduler<alg>::leave_kstack(const std::uint16_t new_sp) {
	_set_SP_register(new_sp);
}
#endif

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

template <scheduling_algorithms alg>
void scheduler<alg>::refresh(void) {
	this->get_current_process().refresh();
}

/**
 * Requests scheduler to reschedule task
 */
extern void watchdog_request(void);

template <scheduling_algorithms alg>
inline void scheduler<alg>::request_preemption(void) {
	watchdog_request();
}

template <scheduling_algorithms alg>
void scheduler<alg>::suspend(void) {
	this->request_preemption();
}

/**
 * Puts a task to sleep on a timer and performs stack manipulation to correctly transfer control to the scheduler
 */

template <scheduling_algorithms alg>
void scheduler<alg>::sleep(const std::size_t ticks) {
	_disable_interrupt();	// Enter critical section

	// Set the sleep counter up for the calling process
	this->get_current_process().sleep(ticks);
	this->request_preemption();

	_enable_interrupt();
}

template <scheduling_algorithms alg>
void scheduler<alg>::block(void) {
	_disable_interrupt();	// Enter critical section

	// Set the blocking flag on the current process
	this->get_current_process().block();
	this->request_preemption();

	_enable_interrupt();
}

template <scheduling_algorithms alg>
void scheduler<alg>::ret(void) {
	_disable_interrupt();	// Enter critical section

	// Set the complete flag on the current process
	this->get_current_process().ret();
	this->request_preemption();

	_enable_interrupt();
}

/**
 * Unblocks a process when requested
 */

template <scheduling_algorithms alg>
void scheduler<alg>::unblock(task &target) {
	target.unblock();
}

#endif
