
/*
 * scheduler.h
 *
 *  Created on: Sep 11, 2019
 *      Author: krad2
 */

#ifndef SCHEDULER_H_
#define SCHEDULER_H_

#include <config.h>
#include <task.h>
#include <scheduler_base.h>

#include <cstdarg>

#include <vector>
#include <initializer_list>
#include <algorithm>

/**
 * Outward-facing scheduler interface - this is meant for use
 */

template <scheduling_algorithms alg>
class scheduler : public base_scheduler<alg> {
public:

	/**
	 * Constructors that mostly call the base class constructors
	 */

	scheduler();
	scheduler(const std::initializer_list<task> &task_list);

	/**
	 * Also calls add / remove in the base class
	 */

	void add_task(const task &t);
	void cleanup(task &t);

	/**
	 * Initializes operating system using configuration set in config.h
	 */

	void init(void);

	/**
	 * Starts OS once initialized or initializes OS and starts it given a list of tasks
	 */

	void start(void);
	void start(const std::initializer_list<task> &task_list);

	/**
	 * Functions handling the context switch process
	 */

	inline void context_switch(void);
	inline void save_context(void);
	inline void restore_context(task &runnable);
//	inline void enter_kernel_mode(void);
//	inline void leave_kernel_mode(void);
	inline void enter_kstack(void);
	inline void leave_kstack(void);

	/**
	 * Function that actually schedules a new task
	 */

	task &schedule(void);

	/**
	 * Returns the current process or the resource monitor for that process
	 */

	inline task &get_current_process(void);
	const thread_info &get_thread_state(void);

	/**
	 * Puts a task to sleep either manually or on a timer
	 */

	__attribute__((noinline)) void sleep(std::size_t ticks);
	__attribute__((noinline)) void block(void);
	inline void unblock(void);
};

#include <scheduler.cpp>

#endif /* SCHEDULER_H_ */
