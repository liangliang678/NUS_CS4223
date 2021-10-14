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

    bus[i].busy = 4;
    bus[i].sender = sender;
    bus[i].tran = tran;
    bus[i].data = data;
    bus[i].addr = addr;
    bus[i].len = len;
    
    pthread_mutex_unlock(&mutex_bus);
    return 1;
}
