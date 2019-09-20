/*
 * task.h
 *
 *  Created on: Sep 6, 2019
 *      Author: krad2
 */

#ifndef TASK_H_
#define TASK_H_

#include <msp430.h>

#include <csetjmp>
#include <cstdint>

#include <memory>
#include <string>

class task;

/**
 * Resource monitoring struct for task
 */

struct thread_info {
	std::uint8_t priority;

	std::size_t stack_size;
	std::size_t stack_usage;

	std::size_t ticks;
	std::size_t sleep_ticks;

	bool blocking;

	const std::string to_string(void);
};

/**
 * Task class representing a process with its own address space, resource monitor, and runnable
 */

class task {
public:

	/**
	 * Constructors for scheduler data structure management
	 */

	task(std::int16_t (*runnable)(void), std::size_t stack_size, std::uint8_t priority = 1);

	/**
	 * Copy-ish constructors
	 */

	task(const task &other);
	task &operator=(const task&other);

	/**
	 * Context switching functions
	 */

	__attribute__((always_inline)) inline void pause(void);
	void load(void);

	/**
	 * Scheduling state management functions
	 */

	void refresh(void);
	void update(void);
	void sleep(const std::size_t ticks);
	void block(void);
	void unblock(void);

	/**
	 * State information retrieval functions
	 */

	std::uint16_t get_latest_sp(void) const;
	std::uint8_t get_priority() const;
	const thread_info &get_state(void) const;
	bool sleeping(void) const;
	bool blocking(void) const;

	/**
	 * Idle hook which is called when no tasks are available to run
	 */

	static std::int16_t idle(void);
	static task idle_hook;

private:
	friend struct cmp_isr_deadline;

	/**
	 * Auto-managed resizable user stack / heap / address space
	 */

	std::unique_ptr<std::uint16_t []> ustack;

	/**
	 * Thread context block
	 */

	std::jmp_buf context;

	/**
	 * Resource monitor struct
	 */

	thread_info info;

	/**
	 * Pointer to process function
	 */

	std::int16_t (*runnable)(void) = nullptr;
};

/**
 * Comparator function for interrupt tasks, allows the wait queue to pick the minimum latency interrupt to handle
 */

struct cmp_isr_deadline {
	bool operator()(const task &t1, const task &t2) {
		return t1.info.priority > t2.info.priority;
	}
};

/**
 * Thread context switching function which saves current task context and performs stack modification to
 * realign stack after an interrupt
 */

__attribute__((always_inline)) inline void task::pause(void) {

	// Save task context
	setjmp(this->context);

	/**
	 * Interrupt pushes PC + SR to stack, must retrieve them, store them in the TCB and unroll the stack
	 */

	// Fetches bytes of pushed PC words
	const register std::uint16_t *stack_top = reinterpret_cast<std::uint16_t *>(__get_SP_register());
	const register std::uint16_t top_bits = (*(stack_top) & 0xF000) >> 12;

	// Rolls back stack to deallocate words
	__set_SP_register(__get_SP_register() + 4);

	// Writes pushed PC from interrupt into TCB as pointer to next normal instruction and SP for proper SP location
	this->context[8] = static_cast<std::uint32_t>(top_bits) << 16 | *(stack_top + 1);
	this->context[7] = __get_SP_register();
}

#endif /* TASK_H_ */
