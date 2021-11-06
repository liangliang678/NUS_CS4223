# CS4223 Project Report

## Introduction

In this assignment we implemented a simulator for cache coherence protocols. We used C for programming, gcc for compiling. The platform is x86_64 GNU/Linux and we used pthread library for shared memory parallel programming. We used shared memory parallel programming because using one thread to simulate one core is more elegant. If we manipulate cache states of 4 cores in one thread the concept of "bus transaction" is not clear and it will probably eliminate randomness of accessing bus.

## Implementation

### Parse Input

### Initiate Simulation

### Bus Structure

### MESI Simulation

Every thread has its private virable `cycle`. Our 4 cores must be in the same cycle at all time, so the synchronization is important. Every time we call `cycle++`, we call `pthread_barrier_wait()` subsequently. In this way every core is in the same cycle, but there is certain overhead. If we do a lot of operations in one cycle, the overhead is relatively small. Because we only simulate cache states, the overhead is obvious and the simulation is slow. But this approch adds more scalability to the simulation program because if we want to simulate more (e.g. register file context), the simulation will be faster than serial program.



## Result

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

## Conclusion
