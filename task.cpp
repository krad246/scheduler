/*
 * task.cpp
 *
 *  Created on: Sep 6, 2019
 *      Author: krad2
 */

#include <task.h>

task task::idle_hook = task(task::idle, 32);

message::message(const task *src, const task *target, void *data) {
	this->sender = src->runnable;
	this->receiver = target->runnable;
	this->data_ptr = data;
}

task::task(std::int16_t (*runnable)(void), std::size_t stack_size, std::uint8_t priority) {
	this->ustack = std::make_unique<std::uint16_t []>(stack_size);
	this->runnable = runnable;
	this->messages = std::queue<message>();
	this->info = {
			.priority = priority,
			.stack_size = stack_size,
			.stack_usage = 0,
			.ticks = 0,
			.sleep_ticks = 0
	};

#ifdef __LARGE_CODE_MODEL__
	this->context[8] = reinterpret_cast<std::uint32_t>(runnable);
	this->context[7] = reinterpret_cast<std::uint32_t>(this->ustack.get() + stack_size);
#else
	this->context[8] = reinterpret_cast<std::uint16_t>(runnable);
	this->context[7] = reinterpret_cast<std::uint16_t>(this->ustack.get() + stack_size);
#endif
}

task::task(const task &other) {
//	this->ustack = std::make_unique<std::uint16_t []>(other.info.stack_size);
//	this->runnable = other.runnable;
//	this->messages = std::queue<message>();
//	this->info = other.info;
//
//#ifdef __LARGE_CODE_MODEL__
//	this->context[8] = reinterpret_cast<std::uint32_t>(this->runnable);
//	this->context[7] = reinterpret_cast<std::uint32_t>(this->ustack.get() + this->info.stack_size);
//#else
//	this->context[8] = reinterpret_cast<std::uint16_t>(runnable);
//	this->context[7] = reinterpret_cast<std::uint16_t>(this->ustack.get() + this->info.stack_size);
//#endif

	new (this) task(other.runnable, other.info.stack_size, other.info.priority);
}

task::task(task &&other) {
	this->ustack = std::move(other.ustack);
	this->runnable = std::move(other.runnable);
	this->messages = std::move(other.messages);
	this->info = std::move(other.info);
	//	this->info = {
//			.priority = priority,
//			.stack_size = stack_size,
//			.stack_usage = 0,
//			.ticks = 0,
//			.sleep_ticks = 0
//	};

#ifdef __LARGE_CODE_MODEL__
	this->context[8] = reinterpret_cast<std::uint32_t>(runnable);
	this->context[7] = reinterpret_cast<std::uint32_t>(this->ustack.get() + this->info.stack_size);
#else
	this->context[8] = reinterpret_cast<std::uint16_t>(runnable);
	this->context[7] = reinterpret_cast<std::uint16_t>(this->ustack.get() + stack_size);
#endif
}

std::int16_t task::idle(void) {
	for (;;) _low_power_mode_0();
	return 0;
}
