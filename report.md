# CS4223 Project Report

## Introduction

In this assignment we implemented a simulator for cache coherence protocols. We used C for programming, gcc for compiling. The platform is x86_64 GNU/Linux and we used pthread library for shared memory parallel programming. We used shared memory parallel programming because using one thread to simulate one core is more elegant. If we manipulate cache states of 4 cores in one thread the concept of "bus transaction" is not clear and it will probably eliminate randomness of accessing bus.

## Implementation

### Parse Input

In function `main()` we parse the input and set varibles `protocol`, `input_file_core*`, `cache_size`, `associativity` and `block_size` accordingly. Then we calculate how many blocks are there in one cache way. We also initialize masks for tag and index as well as calculate how many bits tag and index has. At last, we call `simulate_MESI()` or `simulate_Dragon()` to start simulation.

### Initiate Simulation

Because we only need to simulate states of each cache and bus transactions among caches, we do not need to store data of simulated cache. But we need to store state, tag and lru counter of the simulated cache. At first we malloc memory space for state, tag and lru for each cache and initialize them. Here we initialize state of cache as INVALID in Dragon protocol to present that memory block is not in the cache.

Then we initialize bus, the barrier and mutexes used in following simulation program. And spawn 4 threads using `pthread_create()`. The parameter for the thread is its core number. At last, we call `pthread_join()` to wait child threads exiting and print out simulation cycles and statistics about bus transaction.

### Bus Structure

Type `bus_t` is defined:
``` C
typedef struct bus_t{
    int busy;
    int recv[4];
    int tran;
    uint32_t addr;
    int len;
} bus_t;
```

`busy` indicates whether there is a bus transaction ongoing. `Recv[core_num]` indicates whether core `core_num` has received this bus transaction and has done what it ought to do. `tran` is eithre `BUSRD` or `BUSRDX` or `BUSUPD`. `addr` and `len` is self-explained.

The function `int bus_send(int core_num, int tran, uint32_t addr, int len)` is used to send a bus transaction. We need to ensure that core `core_num` does not has pending bus transaction when calling this function.

The function `int snoop_bus_*(int core_num, int* state, uint32_t* tag, long long* cycle)` is used to check if there are new bus transactions. If yes, state and tag of its core will be modified accordingly. Because dirty data may need to be written back to memory in `snoop_bus_*()`, which takes 100 cycles, we also pass pointer to `cycle` into this function. If we want to disable writing back (because there is only one memory portion for each core), we can set `cycle = NULL`. The return value will be cycle elapsed.

The function `int bus_recv(int core_num)` is used to check if every other cores have received the pending bus transaction. The function `void bus_cancle(int core_num)` is used to complete a pending bus transaction.

To send a bus transaction correctly, we call `bus_send` -> `bus_recv()` -> `bus_cancle()`.

### About Parallel

Every thread has its private virable `cycle`. Our 4 cores must be in the same cycle at all time, so the synchronization is important. Every time we call `cycle++`, we call `pthread_barrier_wait()` subsequently. In this way every core is in the same cycle, but there is certain overhead. If we do a lot of operations in one cycle, the overhead is relatively small. Because we only simulate cache states, the overhead is obvious and the simulation is slow. But this approch adds more scalability to the simulation program because if we want to simulate more (e.g. register file context), the simulation will be faster than serial program.

### MESI Simulation

When a load is issued, we first check whether it hit cache and which way does it hit. If cache miss occurs, we need to send `BUSRD` to bus. If the evicted cache line is dirty, we need to write it back to the main memory before receiving data from main memory and refill the cache line. The state will be `EXCLUSIVE` or `SHARED`. If cache hit, there is not much we need to do. The only thing is to modify lru counter and performance counters.

When a store is issued, we first check whether it hit cache and which way does it hit. If cache miss occurs, we need to send `BUSRDX` to bus. If the evicted cache line is dirty, we need to write it back to the main memory before receiving data from main memory and refill the cache line. The state will be `MODIFIED`. If cache hit the cache line with `SHARED` state, we need to send `BUSRDX` to bus and modify its state to `MODIFIED`. If cache hit the cache line with `EXCLUSIVE` state, we need to modify the state to `MODIFIED` but do not need to send bus transacion.

In `snoop_bus_MESI()`, we first check whether the address in bus transaction hit cache and which way does it hit. If the bus transaction is `BUSRD`, we have following situations:
- cache state is `MODIFIED`: we need to write dirty data back to main memory and turn into `INVALID` state.
- cache state is `EXCLUSIVE`: we need to turn state into `SHARED`.

If the bus transaction is `BUSRDX`, we have following situations:
- cache state is `MODIFIED`: we need to write dirty data back to main memory and turn into `INVALID` state.
- else: we need to turn state into `INVALID`.

### Dragon Simulation
TODO

## Result

We have created a small testbench to check the correctness of our simulation program. In this test bench, at cycle 0, all 4 cores want to load/store the same address. After this, core0 will store to memory blocks which have same index but different tags. So conflict miss will occur in core0.

### Correctness of MESI

After execute following command, we can check the log file under `output` directory and the output infomation in command line.
```shell
./coherence MESI small 8192 2 32
```

Core3 executed first memory instruction first. It sent BUSRDX at cycle 0 and received ack at cycle 1. At cycle 101 the data from memory had arrived at cache and at cycle 102 data_ok is returned to processor.

Core0 executed first memory instruction second. It sent BUSRD at cycle 102, but this time it need to wait until core3 write its dirty data back before reading from memory. So core0 received ack for BUSRD at cycle 203 which means data is up-to-date in memeory. At cycle 303 the data from memory had arrived at cache and at cycle 304 data is returned to processor.

Core1 executed first memory instruction thrid. It sent BUSRD at cycle 303 and received ack at cycle 304. At cycle 404 the data from memory had arrived at cache and at cycle 405 data is returned to processor.

Core2 executed first memory instruction last. It sent BUSRDX at cycle 404 and received ack at cycle 406. In cycle 405 core3 checked for ack before other cores called `snoop_bus()`, so this introduced 1 cycle delay. At cycle 506 the data from memory had arrived at cache and at cycle 507 data_ok is returned to processor.

Core1 continued and caused 2 write-back when replacing. Overall there is 8 bus read, 3 bus write-back and 3 bus invalidation. The result is as expected. But because bus uses first come first serve arbitration policy, the result will vary between test to test.

### Correctness of Dragon

After execute following command, we can check the log file under `output` directory and the output infomation in command line.
```shell
./coherence Dragon small 8192 2 32
```

Core0 executed first memory instruction first. It sent BUSRD at cycle 0 and received ack at cycle 1. At cycle 101 the data from memory had arrived at cache and at cycle 102 data_ok is returned to processor.

Core3 executed first memory instruction second. It sent BUSRD at ctcle 101 and received ack at cycle 103. At cycle 203 the data from memory had arrived at cache and it sent BUDUPD. At cycle 205 data is been updated in core0. At cycle 206 data_ok is returned to processor.

Core2 executed first memory instruction third. It send BUSRD at cycle 206 and receied ack at cycle 307 because it need to wait until dirty data in core3 be written back. At cycle 407 the data had been fetched and it sent BUSUPD, and at cycle 409 data is been updated in core0 and core3. (Acutall at cycle 408 this memory block in core 0 has been replaced by new memory block, but at clcle 407 core0 has been ready to receive the data, so the data is received by core0 but dropped) At cycle 410 data_ok is returned to processor.

Core1 executed first memory instruction last. It sent BUSRD at ctcle 409 and received ack at cycle 511 because it need to wait until dirty data in core2 be written back. At cycle 611 the data had been fetched and at cycle 612 the data had been returned to processor.

Core1 continued and caused 2 write-back when replacing. Overall there is 8 bus read, 4 bus write-back and 12 bytes bus update. The result is as expected. But because bus uses first come first serve arbitration policy, the result will vary between test to test.

### Quantitative Analysis
TODO

## Conclusion
TODO
