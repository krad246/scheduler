# MSP430 Scheduler
This is a preemptive scheduler written for the MSP430G2553. The scheduler currently supports the following scheduling algorithms:

- Round Robin
- Lottery

# Features
- Low memory usage: 32 bytes for each task context + 14 bytes stack overhead for the system / kernel stack.
- Tasks have configurable stack sizes and priority levels.
- Tasks can run to completion and will be freed by the scheduler dynamically.

# Future Work
- Making an actually usable user-mode stack independent to each process in a way that keeps the task context in one place and the user stack in another.