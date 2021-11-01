#ifndef COHERENCE_H
#define COHERENCE_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <pthread.h>
#include "coherence.h"

#define MESI 1
#define DRAGON 2

#define MODIFIED 1
#define EXCLUSIVE 2
#define SHARED 3
#define INVALID 4

#define BUSRD 1
#define BUSRDX 2

typedef struct bus_t{
    int busy;
    int ack;
    int sender;
    int tran;
    char* data;
    uint32_t addr;
    int len;
} bus_t;

extern int protocol;
extern FILE* input_file_core0;
extern FILE* input_file_core1;
extern FILE* input_file_core2;
extern FILE* input_file_core3;
extern int cache_size;
extern int assocaitivity;
extern int block_size;
extern int block_num;
extern uint32_t TAG_INDEX_MASK;
extern uint32_t TAG_MASK;
extern uint32_t INDEX_MASK;
extern uint32_t offset_bits;
extern uint32_t index_bits;
extern bus_t bus[4];
extern pthread_mutex_t mutex_bus;

extern void simulate_MESI();
extern void* MESI_core(int* core_num_pointer);
extern void simulate_Dragon();
extern int check_share(uint32_t tag, uint32_t index);

extern int bus_send(int sender, int tran, char* data, uint32_t addr, int len);
extern void snoop_bus(int core_num, int* state, uint32_t* tag);
extern int bus_ack(int channel);
extern void bus_cancle(int channel);

#endif
