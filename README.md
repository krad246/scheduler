# MSP430 Scheduler
This is a preemptive scheduler written for the MSP430G2553. The scheduler currently supports the following scheduling algorithms:

- Weighted Round Robin: Tasks are run sequentially with a time share approximately equal to the proportion of the priority 'pool' they have.
- Lottery: Similar method to weighted round robin, except pseudorandom so that overall task wait times are smaller.

# Features
- Low memory usage: 32 bytes for each task context.
- Tasks have configurable stack sizes and priority levels.
- Tasks can run to completion and will be freed by the scheduler dynamically.
- Tasks can block / sleep through a `OS::sleep()` call for some specified number of milliseconds.
- 107 us context switch overhead at 16 MHz clock.

# Future Work
- ~~Making an actually usable user-mode stack independent to each process in a way that keeps the task context in one place and the user stack in another.~~
- Converting more of the scheduler to pure assembly.
- Making task stacks dynamically configurable.
- Refactoring the code to clean up code smells - there are a lot of them!
- Implementing the rest of the drivers on the board.