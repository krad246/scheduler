# MSP430 Scheduler
This is a preemptive & cooperative scheduler written for the *entire* MSP430 family, supporting the various code & data models, as well as the CPU-X extension.

# Features

## Scheduling Algorithms
- Weighted Round Robin
- Lottery - **WIP**
- Stride Scheduling - **WIP**

## Low CPU overhead

All measurements below were conducted with the time slice set to 16 ms.

### Round Robin Measurements
| Clock Speed (MHz) | Context Switch Time |
|---|---|
| 1 |  |
| 2 |  |
| 4 |  |
| 8 |  |
| 16 |  |
| 24 |  |


### Lottery Scheduler Measurements
| Clock Speed (MHz) | Context Switch Time |
|---|---|
| 1 |  |
| 2 |  |
| 4 |  |
| 8 |  |
| 16 |  |
| 24 |  |

### Stride Scheduler Measurements
| Clock Speed (MHz) | Context Switch Time |
|---|---|
| 1 |  |
| 2 |  |
| 4 |  |
| 8 |  |
| 16 |  |
| 24 |  |

## Low memory overhead
| Code / Data Model | Memory Usage (B) |
|---|---|
| Small | 36 |
| Large | 58 |

## Configurability
- Tasks have configurable stack sizes and priority levels.

## Dynamic threading
- Supports dynamic thread creation & destruction.
- Automatic memory management

## Blocking, Sleeping & Suspension
- Tasks can block, sleep, and suspend via `OS::suspend()`, `OS::sleep(size_t ticks)`, and `OS::block()`.

## Ease of use
- Provide a `driver_init` function.
- Fill out `functions.cpp`, `config.cpp`, and `config.h`.
- Initialize and start the kernel!

# Future Work
- Implementing system calls.
- Implementing pipes & message queues.
- Implementing hardware abstraction layer.