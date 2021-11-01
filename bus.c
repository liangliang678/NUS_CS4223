#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <pthread.h>
#include "coherence.h"

int bus_send(int sender, int tran, char* data, uint32_t addr, int len)
{
    pthread_mutex_lock(&mutex_bus);
    int i;
    for(i = 0; i < 4; i++){
        if(bus[i].busy == 0){
            break;
        }
    }

    // no free bus channel
    if(i == 4){
        pthread_mutex_unlock(&mutex_bus);
        return 0;
    }

    // exist a bus trans with same mem block
    for(int j = 0; j < 4; j++){
        if(bus[j].busy){
            if(addr & TAG_INDEX_MASK == bus[j].addr & TAG_INDEX_MASK){
                pthread_mutex_unlock(&mutex_bus);
                return 0;
            }
        }
    }

    bus[i].busy = 5;
    bus[i].ack = 0;
    bus[i].sender = sender;
    bus[i].tran = tran;
    bus[i].data = data;
    bus[i].addr = addr;
    bus[i].len = len;
    
    pthread_mutex_unlock(&mutex_bus);
    return 1;
}

void snoop_bus(int core_num, int* state, uint32_t* tag)
{
    pthread_mutex_lock(&mutex_bus);
    for(int i = 0; i < 4; i++){
        if(bus[i].sender != core_num && bus[i].busy > 1){
            uint32_t bus_tag = bus[i].addr >> (offset_bits + index_bits);
            uint32_t bus_index = (bus[i].addr & INDEX_MASK) >> offset_bits;
            int hit_flag = 0;
            for(int i = 0; i < assocaitivity; i++){
                if(bus_tag == tag[i * block_num + bus_index] && state[i * block_num + bus_index] != INVALID){
                    hit_flag = i + 1;
                    break;
                }
            }
            if(!hit_flag){
                bus[i].busy--;
                continue;
            }

            if(bus[i].tran = BUSRD){
                if(state[(hit_flag - 1) * block_num + bus_index] == MODIFIED){
                    bus[i].ack = 1;
                    state[(hit_flag - 1) * block_num + bus_index] = SHARED;
                }
                else if(state[(hit_flag - 1) * block_num + bus_index] == EXCLUSIVE){
                    state[(hit_flag - 1) * block_num + bus_index] = SHARED;
                }
            }
            else if(bus[i].tran = BUSRDX){
                if(state[(hit_flag - 1) * block_num + bus_index] == MODIFIED){
                    bus[i].ack = 1;
                    state[(hit_flag - 1) * block_num + bus_index] = INVALID;
                }
                else{
                    state[(hit_flag - 1) * block_num + bus_index] = INVALID;
                }
            }

            bus[i].busy--;
        }
    }
    
    pthread_mutex_unlock(&mutex_bus);
}

int bus_ack(int channel)
{
    return (bus[channel].busy == 1) ? bus[channel].ack : -1;
}

void bus_cancle(int channel)
{
    bus[channel].busy = 0;
}
