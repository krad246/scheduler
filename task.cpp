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
 * Default constructor
 */

task::task() {
	// No stack allocated yet
	this->ustack = std::unique_ptr<std::uint16_t []>(nullptr);

	// No code associated with the thread yet
	this->runnable = nullptr;

	// Default values for state
	this->info = {
			.id = 0,
			.priority = 0,
			.stack_size = 0,
			.stack_usage = 0,
			.ticks = 0,
			.sleep_ticks = 0,
			.blocked = true,
			.complete = true
	};
}

/**
 * Constructor for task configuration
 * @param runnable - pointer to executable function
 * @param stack_size - size of allocated stack address space
 * @param priority - task run queue priority
 */
static std::uint16_t tid = 1;
task::task(std::int16_t (*runnable)(void), std::size_t stack_size, std::uint8_t priority, bool blocking) {
	// Allocate process stack
	this->ustack = std::make_unique<std::uint16_t []>(stack_size);

	// Initialize runnable to passed-in function
	this->runnable = runnable;

	// Initializes resource monitors to initial conditions
	this->info = {
			.id = tid++,
			.priority = priority,
			.stack_size = stack_size,
			.stack_usage = 0,
			.ticks = 0,
			.sleep_ticks = 0,
			.blocked = blocking,
			.complete = false
	};

	/**
	 * Writes address of executable to PC location of TCB and top of the stack to SP location
	 */

#if defined(__LARGE_CODE_MODEL__) || defined(__LARGE_DATA_MODEL__)
	this->context[8] = reinterpret_cast<std::uint32_t>(runnable);
	this->context[7] = stack_base(this);
#else
	this->context[8] = reinterpret_cast<std::uint16_t>(runnable);
	this->context[7] = stack_base(this);
#endif
}

/**
 * Constructs task from another instance using placement-new
 */

task::task(const task &other) {
	new (this) task(other.runnable, other.info.stack_size, other.info.priority, other.info.blocked);
	this->info = other.info;
}

/**
 * Constructs task from another instance using placement-new
 */

task &task::operator=(const task &other) {
	new (this) task(other.runnable, other.info.stack_size, other.info.priority); // Copy basic task structure first
	this->info = other.info; // Update thread states

	// Copy whole stack (can be optimized)
	std::memcpy(this->ustack.get(), other.ustack.get(), sizeof(other.ustack[0]) * other.info.stack_size);

	// Copy the context
	std::memcpy(&this->context, &other.context, sizeof(other.context));

	// SP is invalid, copy the offset wrt to the base of the stack
	auto dst_stack_base = stack_base(this); // Get MY stack base
	auto src_stack_base = stack_base(&other); // Get THEIR stack base
	std::size_t stack_usage_state = other.get_stack_usage(); // Get the used offset

	// Apply this offset to MY base
	set_task_sp(this, dst_stack_base - stack_usage_state);
	return *this;
}

/**
 * Context switching function which activates a new process
 */

extern void watchdog_suspend(void);
void task::load(void) {
	// Increase run count
	this->info.ticks++;

	// Restore task context and jump to it
	ctx_load(this->context);
}

/**
 * Function that updates task resource monitors
 */

void task::update(void) {
	// Update sleep state and stack usage figure
	if (this->info.sleep_ticks > 0) this->info.sleep_ticks--;

	// Grab the base of the stack and calculate the distance between the last known location of the top and this base
	this->info.stack_usage = this->get_stack_usage();
}

/**
 * Function that reenables scheduler control and updates resource monitors
 */

void task::refresh(void) {
	_disable_interrupt();
#ifdef DEBUG_MODE
	this->info.stack_usage = this->get_stack_usage();
#endif
	_enable_interrupt();
}

/**
 * Updates sleep counter on task
 * @param ticks - number of ticks to wait before reeawakening task
 */

void task::sleep(const std::size_t ticks) {
	if (this->info.sleep_ticks > 0) return;
	else this->info.sleep_ticks = ticks;
}

/**
 * Puts task to sleep until signalled / notified by another thread
 */

void task::block(void) {
	this->info.blocked = true;
}

/**
 * Reawakens a task once signalled / notified
 */

void task::unblock(void) {
	this->info.blocked = false;
}

/**
 * Kills task
 */

void task::ret(void) {
	this->info.complete = true;
}

/**
 * Fetches last known location of the top of the stack
 */

#if defined(__LARGE_CODE_MODEL__) || defined(__LARGE_DATA_MODEL__)
std::uint32_t task::get_task_sp(void) const {
	return this->context[7];
}
#else
std::uint16_t task::get_task_sp(void) const {
	return this->context[7];
}
#endif

std::size_t task::get_stack_usage(void) const {
	return stack_base(this) - this->get_task_sp();
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
	return this->info.blocked;
}

/**
 * Asserts if task is done
 */

bool task::complete(void) const {
	return this->info.complete;
}

#if defined(__LARGE_CODE_MODEL__) || defined(__LARGE_DATA_MODEL__)
std::uint32_t task::stack_base(const task *t) {
	return reinterpret_cast<std::uint32_t>(t->ustack.get() + t->info.stack_size);
}

void task::set_task_sp(task *t, std::uint32_t val) {
	t->context[7] = val;
}

#else
std::uint16_t task::stack_base(const task *t) {
	return reinterpret_cast<std::uint16_t>(t->ustack.get() + t->info.stack_size);
}

void task::set_task_sp(task *t, std::uint16_t val) {
	t->context[7] = val;
}
#endif

/**
 * Idle hook - sets system in low power mode
 */

std::int16_t task::idle(void) {
	for (;;) _low_power_mode_0();
	return 0;
}
