# CS4223 Project Report

## Group Members

Wu Junliang
Do Xuan Long

## Introduction

In this assignment, we implemented a simulator for cache coherence protocols. We used C for programming, gcc for compiling. The platform is x86_64 GNU/Linux and we used pthread library for shared memory parallel programming. We used shared memory parallel programming because using one thread to simulate one core is more elegant. If we manipulate cache states of 4 cores in one thread the concept of "bus transaction" is not clear and it will probably eliminate the randomness of accessing the bus. We implemented MESI protocol and Dragon protocol, and modified Dragon protocol to make it more efficient.

## Implementation

### Parse Input

In function `main()` we parse the input and set variables `protocol`, `input_file_core*`, `cache_size`, `associativity` and `block_size` accordingly. Then we calculate how many blocks are there in one cache way. We also initialize masks for tag and index as well as calculate how many bits tag and index has. At last, we call `simulate_MESI()` or `simulate_Dragon()` to start simulation.

### Initiate Simulation

Because we only need to simulate the states of each cache and bus transactions among caches, we do not need to store data of the simulated cache. But we need to store state, tag and lru counter of the simulated cache. At first, we malloc memory space for state, tag and lru for each cache and initialize them. Here we initialize the state of the cache as INVALID in Dragon protocol to present that the memory block is not in the cache.

Then we initialize the bus, the barrier and mutexes which will be used in the following simulation program. And spawn 4 threads using `pthread_create()`. The parameter for the thread is its core number. At last, we call `pthread_join()` to wait for child threads exiting and print out simulation cycles and statistics about bus transactions.

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

`busy` indicates whether there is a bus transaction ongoing. `recv[core_num]` indicates whether core `core_num` has received this bus transaction and has done what it ought to do. `tran` is either `BUSRD` or `BUSRDX` or `BUSUPD`. `addr` and `len` is self-explained.

The function `int bus_send(int core_num, int tran, uint32_t addr, int len)` is used to send a bus transaction. We need to ensure that core `core_num` does not have pending bus transactions when calling this function.

The function `int snoop_bus_*(int core_num, int* state, uint32_t* tag, long long* cycle)` is used to check if there are new bus transactions. If yes, state and tag of its core will be modified accordingly. Because dirty data may need to be written back to memory in `snoop_bus_*()`, which takes 100 cycles, we also pass the pointer to `cycle` into this function. If we want to disable writing back (because there is only one memory portion for each core), we can set `cycle == NULL`. The return value will be cycle elapsed.

The function `int bus_recv(int core_num)` is used to check if every other core have received the pending bus transaction. The function `void bus_cancle(int core_num)` is used to complete a pending bus transaction.

To send a bus transaction correctly, we call `bus_send()` -> `bus_recv()` -> `bus_cancle()`.

### About Parallel

Every thread has its private variable `cycle`. Our 4 cores must be in the same cycle at all times, so synchronization is important. Every time we call `cycle++`, we call `pthread_barrier_wait()` subsequently. In this way, every core is in the same cycle, but there is a certain overhead. If we do a lot of operations in one cycle, the overhead is relatively small. Because we only simulate cache states, the overhead is obvious and the simulation is slow. But this approach adds more scalability to the simulation program because if we want to simulate more (e.g. register file context), the simulation will be faster than the serial program.

### MESI Simulation

When a load is issued, we first check whether it hit the cache and which way does it hit. If a cache miss occurs, we need to send `BUSRD` to the bus. If the evicted cache line is dirty, we need to write it back to the memory before receiving data from memory and refilling the cache line. The state will be `EXCLUSIVE` or `SHARED`. If a cache hit, there is not much we need to do. The only thing is to modify lru counter and performance counters.

When a store is issued, we first check whether it hit the cache and which way does it hit. If a cache miss occurs, we need to send `BUSRDX` to the bus. If the evicted cache line is dirty, we need to write it back to the main memory before receiving data from memory and refilling the cache line. The state will be `MODIFIED`. If cache hit the cache line with `SHARED` state, we need to send `BUSRDX` to bus and modify its state to `MODIFIED`. If cache hit the cache line with `EXCLUSIVE` state, we need to modify the state to `MODIFIED` but do not need to send bus transaction.

In `snoop_bus_MESI()`, we first check whether the address in the bus transaction hit the cache and which way does it hit. If the bus transaction is `BUSRD`, we have the following situations:
- cache state is `MODIFIED`: we need to write dirty data back to memory and turn it into `INVALID` state.
- cache state is `EXCLUSIVE`: we need to turn the state into `SHARED`.

If the bus transaction is `BUSRDX`, we have the following situations:
- cache state is `MODIFIED`: we need to write dirty data back to memory and turn it into `INVALID` state.
- else: we need to turn the state into `INVALID`.

### Dragon Simulation

Different from lecture slides, We added one more state which is `INVALID` to represent that the block is not in the cache. Initially, all the blocks in cores are assigned `INVALID`. 

When a load is issued and the cache miss occurs, we need to send `BUSRD` to the bus. If the evicted cache line is dirty, we need to write it back to the memory before receiving data from memory and refilling the cache line. Next, if the cache block is not shared, the state transitions to `EXCLUSIVE` (E). Otherwise, the state transitions to state `SHARED CLEAN` (Sc). 

When a load is issued and the cache hit occurs, we do not need to make any changes here since the state of the cache block does not change.

When a write is issued and the cache miss occurs, we need to send `BUSRD` to the bus first. After that, we need to check whether the cache block is shared or not. In the case the cache block is shared, the state transitions to `SHARED MODIFIED` (Sm) and the processor becomes the owner, then we send `BUSUPD` to the bus. Otherwise, the state transitions to `MODIFIED` (M). 

The most complicated case is when a write is issued and the cache hit occurs. 
- If the cache block is in the `MODIFIED` (M) state, there is no transition as the block is not being shared. 
- If the block is in `SHARED MODIFIED` (Sm) state, we have 2 smaller cases. If the shared line is asserted, a `BUSUPD` is generated to update the other cache block. But the shared line is not asserted, the state transitions to `MODIFIED` (M). 
- If the cache block is in the `SHARED CLEAN` (Sc) state and the shared line is asserted, a `BUSUPD` is generated to update the other cache block and the state changes to `SHARED MODIFIED` (Sm). But the shared line is not asserted, the state transitions to `MODIFIED` (M), and no bus transactions are generated.
- If the cache block is in `EXCLUSIVE` (E) state, its state will just be changed to the `MODIFIED` (M) state.
We need to check whether the cache line is asserted because replacing will happen when cache have conflict miss.

P/S: Most of the process stages for Dragon we infer from [Wikipedia Dragon Protocol](https://en.wikipedia.org/wiki/Dragon_protocol).

In `snoop_bus_dragon()`, we first check whether the address in the bus transaction hit the cache and which way does it hit. If the bus transaction is `BUSRD`, we have the following situations:
- cache state is `MODIFIED`: we need to write dirty data back to memory and turn it into `SHARED MODIFIED` state.
- cache state is `SHARED MODIFIED`: we need to write dirty data back to memory.
- cache state is `EXCLUSIVE`: we need to turn the state into `SHARED CLEAN`.

If the bus transaction is `BUSUPD`, we have the following situation:
- cache state is `SHARED MODIFIED`: we need to turn the state into `SHARED CLEAN`.

## Result

We have created a small testbench to check the correctness of our simulation program. In this testbench, at cycle 0, all 4 cores want to load/store the same address. After this, core0 will store into memory blocks that have the same index but different tags. So conflict miss will occur in core0.

### Correctness of MESI

After executing the following command, we can check the log file under `output` directory and the output information in the command line.
```shell
./coherence MESI small 8192 2 32
```

Core3 executed first memory instruction first. It sent BUSRDX at cycle 0 and received ack at cycle 1. At cycle 101 the data from memory had arrived at the cache and at cycle 102 data_ok is returned to the processor.

Core0 executed first memory instruction second. It sent BUSRD at cycle 102, but this time it need to wait until core3 write its dirty data back before reading from memory. So core0 received ack for BUSRD at cycle 203 which means data is up-to-date in memory. At cycle 303 the data from memory had arrived at the cache and at cycle 304 data is returned to the processor.

Core1 executed first memory instruction thrid. It sent BUSRD at cycle 303 and received ack at cycle 304. At cycle 404 the data from memory had arrived at the cache and at cycle 405 data is returned to the processor.

Core2 executed first memory instruction last. It sent BUSRDX at cycle 404 and received ack at cycle 406. In cycle 405 core3 checked for ack before other cores called `snoop_bus()`, so this introduced 1 cycle delay. At cycle 506 the data from memory had arrived at the cache and at cycle 507 data_ok is returned to the processor.

Core1 continued and caused 2 write-back when replacing. Overall there is 8 bus read, 3 bus write-back and 3 bus invalidation. The result is as expected. But because the bus uses a first come first serve arbitration policy, the result will vary from test to test.

### Correctness of Dragon

After executing the following command, we can check the log file under `output` directory and the output information in the command line.
```shell
./coherence Dragon small 8192 2 32
```

Core0 executed first memory instruction first. It sent BUSRD at cycle 0 and received ack at cycle 1. At cycle 101 the data from memory had arrived at the cache and at cycle 102 data_ok is returned to the processor.

Core3 executed first memory instruction second. It sent BUSRD at cycle 101 and received ack at cycle 103. At cycle 203 the data from memory had arrived at the cache and it sent BUDUPD. At cycle 205 data is been updated in core0. At cycle 206 data_ok is returned to the processor.

Core2 executed first memory instruction third. It send BUSRD at cycle 206 and received ack at cycle 307 because it need to wait until dirty data in core3 be written back. At cycle 407 the data had been fetched and it sent BUSUPD, and at cycle 409 data is been updated in core0 and core3. (The tricky part is that at cycle 408 this memory block in core 0 has been replaced by a new memory block, but at cycle 407 core0 has been ready to receive the data, so the data is received by core0 but dropped) At cycle 410 data_ok is returned to the processor.

Core1 executed first memory instruction last. It sent BUSRD at cycle 409 and received ack at cycle 511 because it need to wait until dirty data in core2 be written back. At cycle 611 the data had been fetched and at cycle 612 the data had been returned to the processor.

Core1 continued and caused 2 write-back when replacing. Overall there is 8 bus read, 4 bus write-back and 12 bytes bus update. The result is as expected. But because the bus uses a first come first serve arbitration policy, the result will vary from test to test.

### Quantitative Analysis

Here is the statistic about testbenches, including computing cycles, number of load instructions and number of store instructions of each core. The only thing we need to pay attention to is that in testbench `bodytrack`, there are much fewer memory instructions in core2 compared to other cores.

| program  | core0 compute cycles | core1 compute cycles | core2 compute cycles | core3 compute cycles | core0 load inst | core1 load inst | core2 load inst | core3 load inst | core0 store inst | core1 store inst | core2 store inst | core3 store inst |
| -- | -- | -- | -- | -- | -- | -- | -- | -- | -- | -- | -- | -- |
| blackscholes | 10,430,314 | 10,383,276 | 10,430,338 | 10,394,904 | 1,489,888 | 1,485,857 | 1,492,629 | 1,493,736 | 1,007,461 | 1,004,611 | 1,016,428 | 1,009,391 |
| bodytrack    | 17,729,254 | 17,120,545 | 17,556,877 | 17,140,113 | 2,380,720 | 2,388,005 | 74,523 | 2,416,052 | 889,412 | 899,247 | 43,175 | 908,867 |
| fluidanimate | 11,337,782 | 11,290,799 | 11,337,671 | 11,301,515 | 1,832,392 | 1,821,846 | 1,838,008 | 1,832,174 | 744,111 | 585,998 | 766,181 | 579,291 |

Following results are generated with 8KB 2-way set-associative cache with 32 bytes cache line size. We choose this configuration according to our personal preference.

The following sheet shows the cycles of each core as well as the cycles that the whole simulation takes. MESI protocol and Dragon protocol have similar performances in all three testbenches. But MESI protocol performs slightly better at core0, core1 and core3 in `blackscholes`, and Dragon protocol performs slightly better at core3 in `fluidanimate`.

| program & protocol | core0 cycles | core1 cycles | core2 cycles | core3 cycles | maxinum cycles |
| -- | -- | -- | -- | -- | -- |
| blackscholes MESI   | 15,279,788 | 15,170,174 | 19,862,713 | 15,210,592 | 19,862,713 |
| bodytrack MESI      | 42,643,380 | 42,455,875 | 18,797,112 | 43,311,229 | 43,311,229 |
| fluidanimate MESI   | 42,474,796 | 40,800,710 | 44,969,397 | 44,969,397 | 44,969,397 |
| blackscholes Dragon | 17,224,201 | 17,004,743 | 19,852,434 | 17,176,776 | 19,852,434 |
| bodytrack Dragon    | 42,577,499 | 42,483,009 | 18,794,839 | 43,185,964 | 43,185,964 |
| fluidanimate Dragon | 42,947,535 | 40,390,818 | 45,123,307 | 39,895,892 | 45,123,307 |

The following sheet shows why the core is blocked. Bus idle means that the core is blocked because the bus transaction can not be sent out, or the pending bus transaction has not been received by other cores, or the data is transmitted from cache to cache on the bus. Memory idle means that the core is blocked because the data is being read from the memory or being written to the memory.

In `blackscholes`, MESI protocol has more bus idle but less memory idle. In `bodytrack`, MESI protocol has more bus idle but similar memory idle. In `fluidanimate` MESI protocol has less bus idle and similar memory idle. If the modification to the shared cache line is numerous, Dragon protocol will be less effective. So we can find out that `fluidanimate` has more modification to the shared cache line. Dragon protocol has more memory idle means that many cache lines in `MODIFIED` or `SHARED MODIFIED` state received many `BUSRD` transactions, which asked them to write data back to memory. While in MESI protocol, invalidation of the dirty cache line is relatively rare. We tried to solve this problem of Dragon protocol and the result will be shown later.

| program & protocol | core0 bus idle | core1 bus idle | core2 bus idle | core3 bus idle | core0 memory idle | core1 memory idle | core2 memory idle | core3 memory idle |
| -- | -- | -- | -- | -- | -- | -- | -- | -- |
| blackscholes MESI   | 38,056  | 37,008  | 105,810 | 37,853  | 2,314,600  | 2,260,200  | 6,818,300  | 2,276,100  |
| bodytrack MESI      | 335,647 | 302,008 | 24,583  | 344,422 | 21,314,100 | 21,747,500 | 1,100,700  | 22,508,100 |
| fluidanimate MESI   | 318,837 | 376,560 | 350,732 | 394,105 | 28,243,700 | 26,732,100 | 30,679,100 | 26,156,000 |
| blackscholes Dragon | 34,790  | 30,032  | 101,087 | 37,537  | 4,262,392  | 4,101,672  | 6,812,068  | 4,242,432  |
| bodytrack Dragon    | 304,179 | 298,860 | 20,421  | 321,453 | 21,278,644 | 21,778,040 | 1,109,776  | 22,405,126 |
| fluidanimate Dragon | 406,445 | 439,849 | 349,541 | 400,881 | 28,627,718 | 26,254,946 | 30,835,138 | 25,784,620 |

The following two sheets show the private access versus shared access. In MESI protocol, all store is private access. We mainly focus on private store versus shared store in Dragon protocol. `bodytrack` and `fluidanimate` have much fewer shared store than private store. `blackscholes` have much fewer private store than shared store. The result is not consistent with the above conclusion, which is `fluidanimate` has more modification to the shared cache line. But the BUSUPD generated in `blackscholes` is not many (shown later), the reasonable explanation is that oscillation happened when running `blackscholes`. Cache line was replaced frequently and different performance counters showed inconsistent results.

| program & protocol | core0 private load | core0 shared load | core1 private load | core1 shared load | core2 private load | core2 shared load | core3 private load | core3 shared load |
| -- | -- | -- | -- | -- | -- | -- | -- | -- |
| blackscholes MESI   | 1,015,970 | 473,918 | 1,013,516 | 472,341 | 1,021,312 | 471,317 | 1,018,315 | 475,421 |
| bodytrack MESI      | 1,606,451 | 774,269 | 1,653,393 | 734,612 | 50,528 | 23,995 | 1,605,903 | 810,149 |
| fluidanimate MESI   | 1,395,169 | 437,223 | 1,382,913 | 438,933 | 1,390,413 | 447,595 | 1,395,268 | 436,906 |

| program & protocol | core0 private load | core0 shared load |  core0 private store | core0 shared store | core1 private load | core1 shared load | core1 private store | core1 shared store | core2 private load | core2 shared load | core2 private store | core2 shared store | core3 private load | core3 shared load | core3 private store | core3 shared store |
| -- | -- | -- | -- | -- | -- | -- | -- | -- | -- | -- | -- | -- | -- | -- | -- | -- |
| blackscholes Dragon   | 42,773 | 1,447,115 | 23,015 | 984,446 | 65,806 | 1,420,051 | 45,388 | 959,223 | 1,019,811 | 472,818 | 1,015,664 | 764 | 38,083 | 1,455,653 | 18,318 | 991,073 |
| bodytrack Dragon      | 1,598,069 | 782,651 | 887,397 | 2,015 | 1,613,696 | 774,309 | 898,358 | 889 | 50,396 | 24,127 | 42,726 | 449 | 1,588,032 | 828,020 | 906,584 | 2,283 |
| fluidanimate Dragon   | 1,386,113 | 446,279 | 736,207 | 7,904 | 1,365,192 | 456,654 | 574,125 | 11,873 | 1,389,175 | 448,833 | 764,315 | 1,866 | 1,374,774 | 457,400 | 564,682 | 14,609 |

The following sheet shows the statistics about bus transactions.

| program & protocol | bus read (in times) | bus write-back (in times) | bus invalidation (in times) or bus update (in bytes) |
| -- | -- | -- | -- |
| blackscholes MESI   | 129,646 | 7,047   | 359    | 
| bodytrack MESI      | 582,986 | 83,721  | 1,445  |
| fluidanimate MESI   | 705,272 | 412,842 | 2,219  |
| blackscholes Dragon | 128,787 | 6,574   | 17,676 |
| bodytrack Dragon    | 581,192 | 83,214  | 24,596 |
| fluidanimate Dragon | 699,709 | 407,855 | 49,660 |

The following sheet shows the cache hit rate of each core. Because `blackscholes` is smaller, the hit rate is more than 99%. Dragon protocol is slightly better at cache hit rate because there is no invalidation of cache lines.

| program & protocol | core0 hit rate | core1 hit rate | core2 hit rate | core3 hit rate |
| -- | -- | -- | -- | -- |
| blackscholes MESI   | 0.991197 | 0.991330 | 0.974413 | 0.991263 |
| bodytrack MESI      | 0.942790 | 0.941985 | 0.927917 | 0.940839 |
| fluidanimate MESI   | 0.931636 | 0.929632 | 0.925853 | 0.930912 |
| blackscholes Dragon | 0.991258 | 0.991543 | 0.974431 | 0.991315 |
| bodytrack Dragon    | 0.942924 | 0.942008 | 0.928138 | 0.941215 |
| fluidanimate Dragon | 0.930410 | 0.931545 | 0.926115 | 0.932336 |

## Advanced Task

We discussed before that when a cache line in `MODIFIED` or `SHARED MODIFIED` state received a `BUSRD` transaction, it need to write data back into the memory. Because only when the memory is up-to-date, the cache can read the latest data correctly. But if we can send data from cache to cache, the problem will be easier. And because sending one word only takes 2 cycles, it will be more effective when cache to cache data transmit is permitted. What's more, at most one cache is in `SHARED MODIFIED` state if multiple caches have the same copy, which means we do not need an algorithm to select cache to provide data.

The modified protocol is called "my". The result is summarized in the following sheet. The modified protocol is about 0.1% better than the original Dragon protocol. The progress is more obvious with `fluidanimate` testbench.

| program & protocol | core0 cycles | core1 cycles | core2 cycles | core3 cycles | maxinum cycles |
| -- | -- | -- | -- | -- | -- |
| blackscholes my     | 17,219,670 | 17,002,941 | 19,843,369 | 17,169,329 | 19,843,369 | 
| bodytrack my        | 42,509,007 | 42,444,243 | 18,791,522 | 43,116,508 | 43,116,508 |
| fluidanimate my     | 42,814,508 | 40,270,832 | 45,019,481 | 39,789,821 | 45,019,481 |
| blackscholes Dragon | 17,224,201 | 17,004,743 | 19,852,434 | 17,176,776 | 19,852,434 |
| bodytrack Dragon    | 42,577,499 | 42,483,009 | 18,794,839 | 43,185,964 | 43,185,964 |
| fluidanimate Dragon | 42,947,535 | 40,390,818 | 45,123,307 | 39,895,892 | 45,123,307 |

## Conclusion

Cache coherence is essential in ensuring that the correct data is used, ensuring the correctness of programs. We appreciate the opportunity to this project in the sense that it allows us to explore various problems that come with implementing simulators as well as the snooping mechanisms. 

MESI, being a write invalidate protocol, is more efficient when there are many subsequent writes to the same cache block after an initial write. For these writes to the same block, only one invalidation will be issued, no further bus transactions will be generated for other processors.

Dragon, being a write update protocol, is more efficient when there are many subsequent reads to the same cache block after a write. All other cached values are updated once the writes complete, hence other processors read the same block consecutively no bus transactions will be generated as they already have the values.

As a result, benchmark trace with more consecutive writes to the same cache block after an initial write would see a noticeable lower amount of bus transactions for MESI. Benchmark traces with more consecutive reads on the same cache block would thus produce fewer bus transactions for Dragon.
