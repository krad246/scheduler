/*
 * task.h
 *
 *  Created on: Sep 6, 2019
 *      Author: krad2
 */

#ifndef TASK_H_
#define TASK_H_

#include <msp430.h>

#include <cstdint>

#include <memory>
#include <string>

class task;

#if defined(__LARGE_CODE_MODEL__) || defined(__LARGE_DATA_MODEL__)
	using ctx = std::uint32_t[9];
#else
	using ctx = std::uint16_t[9];
#endif

extern "C" int ctx_save(ctx env);
extern "C" void ctx_load(ctx env);

/**
 * Resource monitoring struct for task
 */

struct thread_info {
	std::uint16_t id;
	std::uint8_t priority;

	std::size_t stack_size;
	std::size_t stack_usage;

	std::size_t ticks;
	std::size_t sleep_ticks;

	bool blocked;
	bool complete;

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
	task();
	task(std::int16_t (*runnable)(void), std::size_t stack_size, std::uint8_t priority = 1, bool blocking = false);

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
	void ret(void);

	std::uint16_t get_tid(void) const;
	std::uint8_t get_priority(void) const;
	std::size_t get_stack_size(void) const;
	std::size_t get_run_count(void) const;
	std::size_t get_stack_usage(void) const;

	bool sleeping(void) const;
	bool blocking(void) const;
	bool complete(void) const;
	const thread_info &get_state(void) const;

	/**
	 * Idle hook which is called when no tasks are available to run
	 */

	static std::int16_t idle(void);
	static task idle_hook;

	/**
	 * Comparator function for interrupt tasks, allows the wait queue to pick the most important interrupt to handle
	 */

	friend bool operator<(const task &t1, const task &t2) {
		return t1.info.priority > t2.info.priority;
	}

	friend bool operator==(const task &t1, const task &t2) {
		return t1.info.id == t2.info.id;
	}

private:

	/**
	 * Auto-managed resizable user stack / heap / address space
	 */

	std::unique_ptr<std::uint16_t []> ustack;

	/**
	 * Thread context block
	 */

	ctx context;

	/**
	 * Resource monitor struct
	 */

	thread_info info;

	/**
	 * Pointer to process function
	 */

	std::int16_t (*runnable)(void) = nullptr;

	/**
	 * State information retrieval functions
	 */

#if defined(__LARGE_CODE_MODEL__) || defined(__LARGE_DATA_MODEL__)
	std::uint32_t get_task_sp(void) const;
	static std::uint32_t stack_base(const task *t);
	static void set_task_sp(task *t, std::uint32_t val);
#else
	std::uint16_t get_task_sp(void) const;
	static std::uint16_t stack_base(const task *t);
	static void set_task_sp(task *t, std::uint16_t val);
#endif
};


/**
 * Thread context switching function which saves current task context and performs stack modification to
 * realign stack after an interrupt
 */

__attribute__((always_inline)) inline void task::pause(void) {

	// Save task context
	ctx_save(this->context);

	/**
	 * Interrupt pushes PC + SR to stack, must retrieve them, store them in the TCB and unroll the stack
	 */

#if defined(__LARGE_CODE_MODEL__) || defined(__LARGE_DATA_MODEL__)
	// Fetches bytes of pushed PC words
	const register std::uint16_t *stack_top = reinterpret_cast<std::uint16_t *>(__get_SP_register());
	const register std::uint16_t top_pc_bits = (*(stack_top) & 0xF000) >> 12;

	// Rolls back stack to deallocate words
	__set_SP_register(__get_SP_register() + 4);

	// Writes pushed PC from interrupt into TCB as pointer to next normal instruction and SP for proper SP location
	this->context[8] = static_cast<std::uint32_t>(top_pc_bits) << 16 | *(stack_top + 1);
	this->context[7] = __get_SP_register();
#else
	// Fetches bytes of pushed PC words
	const register std::uint16_t *stack_top = reinterpret_cast<std::uint16_t *>(__get_SP_register());

	// Rolls back stack to deallocate words
	__set_SP_register(__get_SP_register() + 4);

	// Writes pushed PC from interrupt into TCB as pointer to next normal instruction and SP for proper SP location
	this->context[8] = *(stack_top + 1);
	this->context[7] = __get_SP_register();
#endif
}

#endif /* TASK_H_ */
