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
We added one more keyword state which is `INVALID` to represent that the block is not in the cache. Initially, all the blocks in cores are assigned `INVALID`. 

When a load is issued and the cache miss occurs, we need to send `BUSRD` to bus. If the evicted cache line is dirty, we need to write it back to the main memory before receiving data from main memory and refill the cache line. Next, we check if the cache block is not shared, the state transitions to `EXCLUSIVE` (E). Otherwise, the state transitions to state `SHARED CLEAN` (Sc). 

When a load is issued and the cache miss doesn't occur, we didn't have to make much changes here since the state of the cache block does not change, and retains the value.

When a write is issued and the cache miss occurs then we need to send `BUSRD` to bus first. After that, we need to check whether the cache block is shared or not. In the case the cache block is shared, the state transitions to `SHARED MODIFIED` (Sm) and the processor becomes the owner. Otherwise, the state transitions to `MODIFIED` (M). 

The most comlicated case this when a write is issued and the cache miss doesn't occur. 
- If the cache block is in the `MODIFIED` (M) state, there is no transition as the block is not being shared. 
- If the block is in `SHARED MODIFIED` (Sm) state, we have 2 smaller cases. If the shared line is asserted, a `BUSUPD` is generated to update the other cache block. But the shared line is not asserted, the state transitions to `MODIFIED` (M). 
- If the cache block is in the `SHARED CLEAN` (Sc) state and the shared line is asserted, a `BUSUPD` is generated to update the other cache block and the state changes to `SHARED MODIFIED` (Sm). But the shared line is not asserted, the state transitions to `MODIFIED` (M), and no bus transactions are generated.
- If the cache block is in `EXCLUSIVE` (E) state, its state will just be changed to the `MODIFIED` (M) state.

P/s: Most of the process stages for Dragon we infer from Wikipedia Dragon Protocol: https://en.wikipedia.org/wiki/Dragon_protocol.

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
./coherence MESI blackscholes 8192 2 32
```
> set protocol: MESI
> opening blackscholes_four/*
> successfully opened 4 input files
> cache_size: 8192 Bytes
> associativity: 2
> block_size: 32 Bytes
> simulation init...
> core0 init...
> core1 init...
> core2 init...
> core3 init...

> core1 finish at cycle 15170174
> core1 compute_cycle: 10383276, bus_idle: 37008, mem_idle: 2260200
> core1 load_inst_num: 1485857, store_inst_num: 1004611
> core1 private_acc: 1013516, shared_acc: 472341
> core1 cache_hit: 2468875, cache_miss: 21593, hit_rate: 0.991330

> core3 finish at cycle 15210592
> core3 compute_cycle: 10394904, bus_idle: 37853, mem_idle: 2276100
> core3 load_inst_num: 1493736, store_inst_num: 1009391
> core3 private_acc: 1018315, shared_acc: 475421
> core3 cache_hit: 2481258, cache_miss: 21869, hit_rate: 0.991263
> 
> core0 finish at cycle 15279788
> core0 compute_cycle: 10430314, bus_idle: 38056, mem_idle: 2314600
> core0 load_inst_num: 1489888, store_inst_num: 1007461
> core0 private_acc: 1015970, shared_acc: 473918
> core0 cache_hit: 2475364, cache_miss: 21985, hit_rate: 0.991197

> core2 finish at cycle 19862713
> core2 compute_cycle: 10430338, bus_idle: 105810, mem_idle: 6818300
> core2 load_inst_num: 1492629, store_inst_num: 1016428
> core2 private_acc: 1021312, shared_acc: 471317
> core2 cache_hit: 2444858, cache_miss: 64199, hit_rate: 0.974413

> simulation finished at cycle 19862713.
> bus_rd: 129646, bus_wb: 7047, bus_inv: 359
```

./coherence MESI bodytrack 8192 2 32

```
> set protocol: MESI
> opening bodytrack_four/*
> successfully opened 4 input files
> cache_size: 8192 Bytes
> associativity: 2
> block_size: 32 Bytes
> simulation init...
> core1 init...
> core2 init...
> core3 init...
> core0 init...

> core2 finish at cycle 18797112
> core2 compute_cycle: 17556877, bus_idle: 24583, mem_idle: 1100700
> core2 load_inst_num: 74523, store_inst_num: 43175
> core2 private_acc: 50528, shared_acc: 23995
> core2 cache_hit: 109214, cache_miss: 8484, hit_rate: 0.927917

> core1 finish at cycle 42455875
> core1 compute_cycle: 17120545, bus_idle: 302008, mem_idle: 21747500
> core1 load_inst_num: 2388005, store_inst_num: 899247
> core1 private_acc: 1653393, shared_acc: 734612
> core1 cache_hit: 3096541, cache_miss: 190711, hit_rate: 0.941985

> core0 finish at cycle 42643380
> core0 compute_cycle: 17729254, bus_idle: 335647, mem_idle: 21314100
> core0 load_inst_num: 2380720, store_inst_num: 889412
> core0 private_acc: 1606451, shared_acc: 774269
> core0 cache_hit: 3083048, cache_miss: 187084, hit_rate: 0.942790

> core3 finish at cycle 43311229
> core3 compute_cycle: 17140113, bus_idle: 344422, mem_idle: 22508100
> core3 load_inst_num: 2416052, store_inst_num: 908867
> core3 private_acc: 1605903, shared_acc: 810149
> core3 cache_hit: 3128212, cache_miss: 196707, hit_rate: 0.940839

> simulation finished at cycle 43311229.
> bus_rd: 582986, bus_wb: 83721, bus_inv: 1445
```

./coherence MESI fluidanimate 8192 2 32
```
> set protocol: MESI
> opening fluidanimate_four/*
> successfully opened 4 input files
> cache_size: 8192 Bytes
> associativity: 2
> block_size: 32 Bytes
> simulation init...
> core0 init...
> core1 init...
> core2 init...
> core3 init...

> core3 finish at cycle 40258346
> core3 compute_cycle: 11301515, bus_idle: 394105, mem_idle: 26156000
> core3 load_inst_num: 1832174, store_inst_num: 579291
> core3 private_acc: 1395268, shared_acc: 436906
> core3 cache_hit: 2244861, cache_miss: 166604, hit_rate: 0.930912

> core1 finish at cycle 40800710
> core1 compute_cycle: 11290799, bus_idle: 376560, mem_idle: 26732100
> core1 load_inst_num: 1821846, store_inst_num: 585998
> core1 private_acc: 1382913, shared_acc: 438933
> core1 cache_hit: 2238410, cache_miss: 169434, hit_rate: 0.929632

> core0 finish at cycle 42474796
> core0 compute_cycle: 11337782, bus_idle: 318837, mem_idle: 28243700
> core0 load_inst_num: 1832392, store_inst_num: 744111
> core0 private_acc: 1395169, shared_acc: 437223
> core0 cache_hit: 2400363, cache_miss: 176140, hit_rate: 0.931636

> core2 finish at cycle 44969397
> core2 compute_cycle: 11337671, bus_idle: 350732, mem_idle: 30679100
> core2 load_inst_num: 1838008, store_inst_num: 766181
> core2 private_acc: 1390413, shared_acc: 447595
> core2 cache_hit: 2411095, cache_miss: 193094, hit_rate: 0.925853

> simulation finished at cycle 44969397.
> bus_rd: 705272, bus_wb: 412842, bus_inv: 2219
```
## Conclusion
Cache coherence is essential in ensuring that the correct data is used, ensuring correctness of programs. We appreciate the opportinity to this project in the sense that it allows us explore various problems that come with implementing simulators as well as the snooping mechanisms. 

MESI, being a write invalidate protocol, is more efficient when there are many subsequent writes to the same cache block after an initial write. For these writes to the same block, only one invalidation will be issued, no further bus transactions will be generated for other processors.

Dragon, being a write update protocol, is more efficient when there are many subsequent reads to the same cache block after a write. All other cached values are updated once the writes complete, hence other processors read the same block consecutively no bus transactions will be generated as they already have the values.

As a result, benchmark trace with more consecutive writes to the same cache block after an initial write would see a noticeable lower amount of bus transactions for MESI. Benchmark traces with more consecutive reads on the same cache block would thus produce fewer bus transactions for Dragon.

