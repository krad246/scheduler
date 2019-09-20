/*
 * task.cpp
 *
 *  Created on: Sep 6, 2019
 *      Author: krad2
 */

#include <task.h>
#include <scheduler.h>

const std::string thread_info::to_string(void) {
	std::string msg;
	msg += "-----------\n\r";
#ifdef DEBUG_MODE
	msg += "Name: ";
#else
#endif
	msg += "Priority: " + std::to_string(this->priority) + "\n\r";
	msg += "Stack Size: " + std::to_string(this->stack_size) + "\n\r";
	msg += "Stack Usage: " + std::to_string(this->stack_usage) + "\n\r";
	msg += "Times Run: " + std::to_string(this->ticks) + "\n\r";
	return msg;
}

/**
 * Idle task - sets system in low power mode
 */

task task::idle_hook = task(task::idle, 32);

/**
 * Constructor for task configuration
 * @param runnable - pointer to executable function
 * @param stack_size - size of allocated stack address space
 * @param priority - task run queue priority
 */

task::task(std::int16_t (*runnable)(void), std::size_t stack_size, std::uint8_t priority) {
	// Allocate process stack
	this->ustack = std::make_unique<std::uint16_t []>(stack_size);

	// Initialize runnable to passed-in function
	this->runnable = runnable;

	// Initializes resource monitors to initial conditions
	this->info = {
			.priority = priority,
			.stack_size = stack_size,
			.stack_usage = 0,
			.ticks = 0,
			.sleep_ticks = 0
	};

	/**
	 * Writes address of executable to PC location of TCB and top of the stack to SP location
	 */

#ifdef __LARGE_CODE_MODEL__
	this->context[8] = reinterpret_cast<std::uint32_t>(runnable);
	this->context[7] = reinterpret_cast<std::uint32_t>(this->ustack.get() + stack_size);

#else
	this->context[8] = reinterpret_cast<std::uint16_t>(runnable);
	this->context[7] = reinterpret_cast<std::uint16_t>(this->ustack.get() + stack_size);
#endif
}

/**
 * Constructs task from another instance using placement-new
 */

task::task(const task &other) {
	new (this) task(other.runnable, other.info.stack_size, other.info.priority);
}

/**
 * Constructs task from another instance using placement-new
 */

task &task::operator=(const task &other) {
	new (this) task(other.runnable, other.info.stack_size, other.info.priority);
	return *this;
}

/**
 * Context switching function which activates a new process
 */

void task::load(void) {
	// Increase run count
	this->info.ticks++;

	// Restore task context and jump to it
	longjmp(this->context, 1);
}

/**
 * Function that updates task resource monitors
 */

void task::update(void) {
	// Update sleep state and stack usage figure
	if (this->info.sleep_ticks > 0) this->info.sleep_ticks--;

	// Grab the base of the stack and calculate the distance between the last known location of the top and this base
	this->info.stack_usage = this->ustack.get() + this->info.stack_size - reinterpret_cast<std::uint16_t *>(this->get_latest_sp());
}

/**
 * Function that reenables scheduler control
 */

void task::refresh(void) {
	_enable_interrupt();

#ifdef DEBUG_MODE
	this->info.stack_usage = this->ustack.get() + this->info.stack_size - reinterpret_cast<std::uint16_t *>(_get_SP_register());
#endif
}

/**
 * Updates sleep counter on task
 * @param ticks - number of ticks to wait before reeawakening task
 */

void task::sleep(const std::size_t ticks) {
	this->info.sleep_ticks = ticks;
}

/**
 * Puts task to sleep until signalled / notified by another thread
 */

void task::block(void) {
	this->info.blocking = true;
}

/**
 * Reawakens a task once signalled / notified
 */

void task::unblock(void) {
	this->info.blocking = false;
}

/**
 * Fetches last known location of the top of the stack
 */

std::uint16_t task::get_latest_sp(void) const {
	return this->context[7];
}

/**
 * Fetches task priority
 */

std::uint8_t task::get_priority() const {
	return this->info.priority;
}

/**
 * Fetches resource monitor struct
 */

const thread_info &task::get_state() const {
	return this->info;
}

/**
 * Asserts if task currently has nonzero sleep counts (and is asleep)
 */

bool task::sleeping(void) const {
	return this->info.sleep_ticks > 0;
}

/**
 * Asserts if task has been put to sleep
 */

bool task::blocking(void) const {
	return this->info.blocking;
}

/**
 * Idle hook - sets system in low power mode
 */

std::int16_t task::idle(void) {
	for (;;) _low_power_mode_0();
	return 0;
}
