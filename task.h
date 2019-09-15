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
#include <queue>

class task;

struct message {
	message(const task *src, const task *target, void *data);

	std::int16_t (*sender)(void) = nullptr;
	std::int16_t (*receiver)(void) = nullptr;

	void *data_ptr = nullptr;
};

struct thread_info {
	std::uint8_t priority;

	std::size_t stack_size;
	std::size_t stack_usage;

	std::size_t ticks;
	std::size_t sleep_ticks;

	bool blocking;
};

class task {
public:
	task(std::int16_t (*runnable)(void), std::size_t stack_size, std::uint8_t priority = 1);
	explicit task(const task &other);
	task(task &&other);

	inline void pause(void);
	inline void load(void);

	inline void update(void);
	inline void sleep(const std::size_t ticks);
	inline void block(void);
	inline void unblock(void);

	inline std::uint16_t get_latest_sp(void) const;
	inline std::uint8_t get_priority() const;
	inline bool sleeping(void) const;
	inline bool blocking(void) const;

	static std::int16_t idle(void);
	static task idle_hook;

private:
	friend class message;

	std::unique_ptr<std::uint16_t []> ustack;
	std::jmp_buf context;

	thread_info info;
	std::queue<message> messages;

	std::int16_t (*runnable)(void) = nullptr;
};

#pragma FUNC_ALWAYS_INLINE
inline void task::pause(void) {
	__disable_interrupt();

	// save context
	setjmp(this->context);

	// must unroll pushed interrupt registers
	// place pc value in slot 8 and unrolled sp in slot 7

	// grab upper byte of program counter
	const register std::uint16_t *stack_top = reinterpret_cast<std::uint16_t *>(__get_SP_register());
	const register std::uint16_t top_bits = (*(stack_top) & 0xF000) >> 12;

	// roll back stack pointer to deallocate interrupt words
	__set_SP_register(__get_SP_register() + 4);

	// combine upper byte of pc with rest of pc and put correct stack pointer in context block
	this->context[8] = static_cast<std::uint32_t>(top_bits) << 16 | *(stack_top + 1);
	this->context[7] = __get_SP_register();
}

#pragma FUNC_ALWAYS_INLINE
inline void task::load(void) {
	// increase run count
	this->info.ticks++;

	//
	__enable_interrupt();
	longjmp(this->context, 1);
}

inline void task::update(void) {
	if (this->info.sleep_ticks > 0) {
		this->info.sleep_ticks--;
	}

	this->info.stack_usage = this->ustack.get() + this->info.stack_size - reinterpret_cast<std::uint16_t *>(this->get_latest_sp()); // is this working?
}

inline void task::block(void) {
	this->info.blocking = true;
}

inline void task::sleep(const std::size_t ticks) {
	__disable_interrupt();
	this->info.sleep_ticks = ticks;
}

inline void task::unblock(void) {
	this->info.blocking = false;
}

inline std::uint16_t task::get_latest_sp(void) const {
	return this->context[7];
}

inline std::uint8_t task::get_priority() const {
	return this->info.priority;
}

inline bool task::sleeping(void) const {
	return this->info.sleep_ticks > 0;
}

inline bool task::blocking(void) const {
	return this->info.blocking;
}


#endif /* TASK_H_ */
