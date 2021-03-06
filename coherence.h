#ifndef COHERENCE_H
#define COHERENCE_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <pthread.h>
#include <assert.h>

#define MESI 1
#define DRAGON 2
#define MY 3

#define MODIFIED 1
#define EXCLUSIVE 2
#define SHARED 3
#define INVALID 4

#define BUSRD 1
#define BUSRDX 2
#define BUSUPD 3

#define E 5
#define Sc 6
#define Sm 7
#define M 8

typedef struct bus_t{
    int busy;
    int recv[4];
    int tran;
    uint32_t addr;
    int len;
} bus_t;

extern int protocol;
extern FILE* input_file_core0;
extern FILE* input_file_core1;
extern FILE* input_file_core2;
extern FILE* input_file_core3;
extern int cache_size;
extern int associativity;
extern int block_size;
extern int block_num;
extern uint32_t TAG_INDEX_MASK;
extern uint32_t TAG_MASK;
extern uint32_t INDEX_MASK;
extern uint32_t offset_bits;
extern uint32_t index_bits;

extern pthread_barrier_t barrier;

extern bus_t bus[4];
extern pthread_mutex_t mutex_bus;
extern long long bus_rd;
extern long long bus_wb;
extern long long bus_inv;
extern long long bus_upd;

extern void simulate_MESI();
extern void* MESI_core(void* core_num_pointer);
extern void simulate_Dragon();
extern void* DRAGON_core(void* core_num_pointer);
extern void simulate_my();
extern void* my_core(void* core_num_pointer);

// send a transaction to bus
extern int bus_send(int core_num, int tran, uint32_t addr, int len);
// snoop the bus (MESI) to check if there is new transactions
extern int snoop_bus_MESI(int core_num, int* state, uint32_t* tag, long long* cycle);
// snoop the bus (dragon) to check if there is new transactions
extern int snoop_bus_dragon(int core_num, int* state, uint32_t* tag, long long* cycle);
// snoop the bus (my) to check if there is new transactions
extern int snoop_bus_my(int core_num, int* state, uint32_t* tag, long long* cycle);
// check if all other cores have received transaction it sent
extern int bus_recv(int core_num);
extern int bus_recv_my(int core_num);
// the transaction has been completed
extern void bus_cancle(int core_num);

#endif
