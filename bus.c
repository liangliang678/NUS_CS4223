#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <pthread.h>
#include <assert.h>
#include "coherence.h"

bus_t bus[4];
pthread_mutex_t mutex_bus;
long long bus_wb = 0;
long long bus_inv = 0;

// when calling bus_send, the bus of its core must be free
int bus_send(int core_num, int tran, uint32_t addr, int len)
{
    pthread_mutex_lock(&mutex_bus);

    assert(bus[core_num].busy == 0);

    // exist a bus trans refering same mem block
    for(int j = 0; j < 4; j++){
        if(bus[j].busy){
            if((addr & TAG_INDEX_MASK) == (bus[j].addr & TAG_INDEX_MASK)){
                pthread_mutex_unlock(&mutex_bus);
                return 0;
            }
        }
    }

    bus[core_num].busy = 1;
    bus[core_num].recv[0] = 0;
    bus[core_num].recv[1] = 0;
    bus[core_num].recv[2] = 0;
    bus[core_num].recv[3] = 0;
    bus[core_num].recv[core_num] = 1;
    bus[core_num].tran = tran;
    bus[core_num].addr = addr;
    bus[core_num].len = len;
    
    pthread_mutex_unlock(&mutex_bus);
    return 1;
}

// when calling snoop_bus, it may write dirty cache line back to mem and it takes 100 cycles (ret will be 1)
// if you do not want it to write back, set cycle to NULL
int snoop_bus(int core_num, int* state, uint32_t* tag, long long* cycle)
{
    pthread_mutex_lock(&mutex_bus);
    int ret = 0;
    for(int i = 0; i < 4; i++){
        if(i != core_num && bus[i].busy && bus[i].recv[core_num] == 0){
            uint32_t bus_tag = bus[i].addr >> (offset_bits + index_bits);
            uint32_t bus_index = (bus[i].addr & INDEX_MASK) >> offset_bits;
            int hit_flag = 0;
            for(int i = 0; i < associativity; i++){
                if(bus_tag == tag[i * block_num + bus_index] && state[i * block_num + bus_index] != INVALID){
                    hit_flag = i + 1;
                    break;
                }
            }
            if(!hit_flag){
                bus[i].recv[core_num] = 1;
                continue;
            }
            if(bus[i].tran == BUSRD){
                if(state[(hit_flag - 1) * block_num + bus_index] == MODIFIED){
                    if(cycle){
                        int counter = 100;
                        pthread_mutex_unlock(&mutex_bus);
                        while(counter != 0){
                            (*cycle)++;
                            counter--;
                            pthread_barrier_wait(&barrier);
                        }
                        pthread_mutex_lock(&mutex_bus);
                        state[(hit_flag - 1) * block_num + bus_index] = SHARED;
                        bus_wb++;
                        bus[i].recv[core_num] = 1;
                        ret = 1;
                    }
                }
                else if(state[(hit_flag - 1) * block_num + bus_index] == EXCLUSIVE){
                    state[(hit_flag - 1) * block_num + bus_index] = SHARED;
                    bus[i].recv[core_num] = 1;
                }
                else{
                    bus[i].recv[core_num] = 1;
                }
            }
            else if(bus[i].tran == BUSRDX){
                if(state[(hit_flag - 1) * block_num + bus_index] == MODIFIED){
                    if(cycle){
                        int counter = 100;
                        pthread_mutex_unlock(&mutex_bus);
                        while(counter != 0){
                            (*cycle)++;
                            counter--;
                            pthread_barrier_wait(&barrier);
                        }
                        pthread_mutex_lock(&mutex_bus);
                        state[(hit_flag - 1) * block_num + bus_index] = INVALID;
                        bus_wb++;
                        bus_inv++;
                        bus[i].recv[core_num] = 1;
                        ret = 1;
                    }
                }
                else{
                    state[(hit_flag - 1) * block_num + bus_index] = INVALID;
                    bus[i].recv[core_num] = 1;
                    bus_inv++;
                }
            }
        }
    }
    pthread_mutex_unlock(&mutex_bus);
    return ret;
}

int snoop_bus_dragon(int core_num, int* state, uint32_t* tag, long long* cycle)
{
    pthread_mutex_lock(&mutex_bus);
    int ret = 0;
    for(int i = 0; i < 4; i++){
        if(i != core_num && bus[i].busy && bus[i].recv[core_num] == 0){
            uint32_t bus_tag = bus[i].addr >> (offset_bits + index_bits);
            uint32_t bus_index = (bus[i].addr & INDEX_MASK) >> offset_bits;
            int hit_flag = 0;
            for(int i = 0; i < associativity; i++){
                if(bus_tag == tag[i * block_num + bus_index] && state[i * block_num + bus_index] != INVALID){
                    hit_flag = i + 1;
                    break;
                }
            }
            if(!hit_flag){
                bus[i].recv[core_num] = 1;
                continue;
            }
            if(bus[i].tran == BUSRD){
                if(state[(hit_flag - 1) * block_num + bus_index] == M){
                    if(cycle){
                        int counter = 100;
                        pthread_mutex_unlock(&mutex_bus);
                        while(counter != 0){
                            (*cycle)++;
                            counter--;
                            pthread_barrier_wait(&barrier);
                        }
                        pthread_mutex_lock(&mutex_bus); // Update the main memory
                        state[(hit_flag - 1) * block_num + bus_index] = Sm;
                        bus_wb++;
                        bus[i].recv[core_num] = 1;
                        ret = 1;
                    }
                }else if(state[(hit_flag - 1) * block_num + bus_index] == Sm){
                    if(cycle){
                        int counter = 100;
                        pthread_mutex_unlock(&mutex_bus);
                        while(counter != 0){
                            (*cycle)++;
                            counter--;
                            pthread_barrier_wait(&barrier);
                        }
                        pthread_mutex_lock(&mutex_bus); // Update the main memory
                        state[(hit_flag - 1) * block_num + bus_index] = Sm;
                        bus_wb++;
                        bus[i].recv[core_num] = 1;
                        ret = 1;
                    }
                }else if(state[(hit_flag - 1) * block_num + bus_index] == Sc || state[(hit_flag - 1) * block_num + bus_index] == E) {
                    state[(hit_flag - 1) * block_num + bus_index] = Sc;
                    bus[i].recv[core_num] = 1;
                }
                else{
                    bus[i].recv[core_num] = 1;
                }
            }
            else if(bus[i].tran == BUSUPD){
                if(state[(hit_flag - 1) * block_num + bus_index] == Sm){
                    state[(hit_flag - 1) * block_num + bus_index] = Sc;
                    bus[i].recv[core_num] = 1;
                }
                else if(state[(hit_flag - 1) * block_num + bus_index] == Sc){
                    if(cycle){
                        int counter = int(bus[i].len/4 * 2);
                        pthread_mutex_unlock(&mutex_bus);
                        while(counter != 0){
                            (*cycle)++;
                            counter--;
                            pthread_barrier_wait(&barrier);
                        }
                        pthread_mutex_lock(&mutex_bus); // Update the main memory
                        bus[i].recv[core_num] = 1;
                        ret = 1;
                    }
                }
            }
        }
    }
    pthread_mutex_unlock(&mutex_bus);
    return ret;
}

int bus_recv(int core_num)
{
    return (bus[core_num].recv[0] && bus[core_num].recv[1] && bus[core_num].recv[2] && bus[core_num].recv[3]);
}

void bus_cancle(int core_num)
{
    bus[core_num].busy = 0;
    bus[core_num].recv[0] = 0;
    bus[core_num].recv[1] = 0;
    bus[core_num].recv[2] = 0;
    bus[core_num].recv[3] = 0;
}
