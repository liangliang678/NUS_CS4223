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

`busy` indicates whether there is a bus transaction ongoing. `Recv[core_num]` indicates whether core `core_num` has received this bus transaction and has done what it ought to be done. `tran` is eithre `BUSRD` or `BUSRDX` or `BUSUPD`. `addr` and `len` is self-explained.

The function `int bus_send(int core_num, int tran, uint32_t addr, int len)` is used to send a bus transaction. We need to ensure that core `core_num` does not has pending bus transaction when calling this function.

The function `int snoop_bus_*(int core_num, int* state, uint32_t* tag, long long* cycle)` is used to check if there are new bus transactions. If yes, state and tag of its core will be modified accordingly. Because dirty data may need to be written back to memory in `snoop_bus_*()`, which takes 100 cycles, we also pass pointer to `cycle` into this function. If we want to disable writing back (because there is only one memory portion for each core), we can set `cycle = NULL`.

The function `int bus_recv(int core_num)` is used to check if every other cores have received the pending bus transaction. The function `void bus_cancle(int core_num)` is used to complete a pending bus transaction.

To send a bus transaction correctly, we call `bus_send` -> `bus_recv()` -> `bus_cancle()`.

### About Parallel

Every thread has its private virable `cycle`. Our 4 cores must be in the same cycle at all time, so the synchronization is important. Every time we call `cycle++`, we call `pthread_barrier_wait()` subsequently. In this way every core is in the same cycle, but there is certain overhead. If we do a lot of operations in one cycle, the overhead is relatively small. Because we only simulate cache states, the overhead is obvious and the simulation is slow. But this approch adds more scalability to the simulation program because if we want to simulate more (e.g. register file context), the simulation will be faster than serial program.

### MESI Simulation

When a load is issued, we first check whether it hit cache and which way does it hit. If cache miss occurs, we need to send `BUSRD` to bus. If the evicted cache line is dirty, we need to write it back to the main memory before receiving data from main memory and refill the cache line.

If cache hit, there is not much we need to do. The only thing is to modify lru counter and performance counters.

When a store is issued, we first check whether it hit cache and which way does it hit. If cache miss occurs, we need to send `BUSRDX` to bus. If the evicted cache line is dirty, we need to write it back to the main memory before receiving data from main memory and refill the cache line.

If cache hit the cache line with `SHARED` state, we need to send `BUSRDX` to bus and modify its state to `MODIFIED`. If cache hit the cache line with `EXCLUSIVE` state, we need to modify the state to `MODIFIED` but do not need to send bus transacion.

In `snoop_bus_MESI()`, we first check whether the address in bus transaction hit cache and which way does it hit. If the bus transaction is `BUSRD`, we have following situations:
- cache state is `MODIFIED`: we need to write dirty data back to main memory and turn into `INVALID` state.
- cache state is `EXCLUSIVE`: we need to turn state into `SHARED`.

If the bus transaction is `BUSRDX`, we have following situations:
- cache state is `MODIFIED`: we need to write dirty data back to main memory and turn into `INVALID` state.
- else: we need to turn state into `INVALID`.

### Dragon Simulation
TODO

## Result
TODO

./coherence MESI bodytrack 8192 2 32

```
> set protocol: MESI
> opening bodytrack_four/*
> successfully opened 4 input files
> cache_size: 8192 Bytes
> associativity: 2
> block_size: 32 Bytes
> simulation init...
> core0 init...
> core1 init...
> core2 init...
> core3 init...

> core2 finish at cycle 18797172
> core2 compute_cycle: 17556877, bus_idle: 24370, mem_idle: 1094200
> core2 load_inst_num: 74523, store_inst_num: 43175
> core2 private_acc: 50584, shared_acc: 23939
> core2 cache_hit: 109212, cache_miss: 8486, hit_rate: 0.927900

> core1 finish at cycle 42471023
> core1 compute_cycle: 17120545, bus_idle: 310712, mem_idle: 21733700
> core1 load_inst_num: 2388005, store_inst_num: 899247
> core1 private_acc: 1644258, shared_acc: 743747
> core1 cache_hit: 3096487, cache_miss: 190765, hit_rate: 0.941968
> 
> core0 finish at cycle 42650765
> core0 compute_cycle: 17729254, bus_idle: 339415, mem_idle: 21250400
> core0 load_inst_num: 2380720, store_inst_num: 889412
> core0 private_acc: 1619538, shared_acc: 761182
> core0 cache_hit: 3083020, cache_miss: 187112, hit_rate: 0.942782

> core3 finish at cycle 43244565
> core3 compute_cycle: 17140113, bus_idle: 356993, mem_idle: 22373400
> core3 load_inst_num: 2416052, store_inst_num: 908867
> core3 private_acc: 1595793, shared_acc: 820259
> core3 cache_hit: 3128834, cache_miss: 196085, hit_rate: 0.941026

> simulation finished at cycle 43244565.
> bus_wb: 83571, bus_inv: 1466
```

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

> core1 finish_MESI at cycle 15171802
> core1 compute_cycle: 10383276, bus_idle: 36570, mem_idle: 2256000
> core1 load_inst_num: 1485857, store_inst_num: 1004611
> core1 private_acc: 1013546, shared_acc: 472311
> core1 cache_hit: 2468853, cache_miss: 21615, hit_rate: 0.991321

> core3 finish_MESI at cycle 15215710
> core3 compute_cycle: 10394904, bus_idle: 39587, mem_idle: 2271200
> core3 load_inst_num: 1493736, store_inst_num: 1009391
> core3 private_acc: 1018319, shared_acc: 475417
> core3 cache_hit: 2481221, cache_miss: 21906, hit_rate: 0.991249

> core0 finish_MESI at cycle 15290915
> core0 compute_cycle: 10430314, bus_idle: 41327, mem_idle: 2314900
> core0 load_inst_num: 1489888, store_inst_num: 1007461
> core0 private_acc: 1016006, shared_acc: 473882
> core0 cache_hit: 2475314, cache_miss: 22035, hit_rate: 0.991177

> core2 finish_MESI at cycle 19872765
> core2 compute_cycle: 10430338, bus_idle: 111412, mem_idle: 6808700
> core2 load_inst_num: 1492629, store_inst_num: 1016428
> core2 private_acc: 1021367, shared_acc: 471262
> core2 cache_hit: 2444838, cache_miss: 64219, hit_rate: 0.974405

> simulation finish_MESIed at cycle 19872765.
> bus_wb: 7089, bus_inv: 386
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
> core2 init...
> core1 init...
> core3 init...

> core3 finish_MESI at cycle 40267143
> core3 compute_cycle: 11301515, bus_idle: 384592, mem_idle: 26101000
> core3 load_inst_num: 1832174, store_inst_num: 579291
> core3 private_acc: 1394766, shared_acc: 437408
> core3 cache_hit: 2244752, cache_miss: 166713, hit_rate: 0.930867

> core1 finish_MESI at cycle 40804747
> core1 compute_cycle: 11290799, bus_idle: 384830, mem_idle: 26649000
> core1 load_inst_num: 1821846, store_inst_num: 585998
> core1 private_acc: 1382674, shared_acc: 439172
> core1 cache_hit: 2238437, cache_miss: 169407, hit_rate: 0.929644

> core0 finish_MESI at cycle 42481572
> core0 compute_cycle: 11337782, bus_idle: 323257, mem_idle: 28211300
> core0 load_inst_num: 1832392, store_inst_num: 744111
> core0 private_acc: 1394849, shared_acc: 437543
> core0 cache_hit: 2400349, cache_miss: 176154, hit_rate: 0.931631

> core2 finish_MESI at cycle 44983715
> core2 compute_cycle: 11337671, bus_idle: 353698, mem_idle: 30654200
> core2 load_inst_num: 1838008, store_inst_num: 766181
> core2 private_acc: 1390218, shared_acc: 447790
> core2 cache_hit: 2411032, cache_miss: 193157, hit_rate: 0.925828

> simulation finish_MESIed at cycle 44983715.
> bus_wb: 412950, bus_inv: 2175
```
## Conclusion
