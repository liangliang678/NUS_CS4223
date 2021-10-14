#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <pthread.h>
#include "coherence.h"

int* state_core0;
int* state_core1;
int* state_core2;
int* state_core3;
uint32_t* tag_core0;
uint32_t* tag_core1;
uint32_t* tag_core2;
uint32_t* tag_core3;
int* lru_core0;
int* lru_core1;
int* lru_core2;
int* lru_core3;

pthread_barrier_t barrier;
pthread_mutex_t mutex_bus;
pthread_mutex_t mutex_finish;

int finish = 0;
bus_t bus[4];

void simulate_MESI()
{
    printf("> simulation init...\n");
    state_core0 = (int*)malloc(assocaitivity * block_num * sizeof(int));
    state_core1 = (int*)malloc(assocaitivity * block_num * sizeof(int));
    state_core2 = (int*)malloc(assocaitivity * block_num * sizeof(int));
    state_core3 = (int*)malloc(assocaitivity * block_num * sizeof(int));
    tag_core0 = (int*)malloc(assocaitivity * block_num * sizeof(uint32_t));
    tag_core1 = (int*)malloc(assocaitivity * block_num * sizeof(uint32_t));
    tag_core2 = (int*)malloc(assocaitivity * block_num * sizeof(uint32_t));
    tag_core3 = (int*)malloc(assocaitivity * block_num * sizeof(uint32_t));
    lru_core0 = (int*)malloc(assocaitivity * block_num * sizeof(int));
    lru_core1 = (int*)malloc(assocaitivity * block_num * sizeof(int));
    lru_core2 = (int*)malloc(assocaitivity * block_num * sizeof(int));
    lru_core3 = (int*)malloc(assocaitivity * block_num * sizeof(int));

    for(int i = 0; i < assocaitivity; i++){
        for(int ii = 0; ii < block_num; ii++){
            state_core0[i * block_num + ii] = INVALID;
            state_core1[i * block_num + ii] = INVALID;
            state_core2[i * block_num + ii] = INVALID;
            state_core3[i * block_num + ii] = INVALID;
            tag_core0[i * block_num + ii] = 0;
            tag_core1[i * block_num + ii] = 0;
            tag_core2[i * block_num + ii] = 0;
            tag_core3[i * block_num + ii] = 0;
            lru_core0[i * block_num + ii] = 0;
            lru_core1[i * block_num + ii] = 0;
            lru_core2[i * block_num + ii] = 0;
            lru_core3[i * block_num + ii] = 0;
        }
    }

    for(int i = 0; i < 4; i++){
        bus[i].busy = 0;
        bus[i].sender = -1;
        bus[i].tran = 0;
        bus[i].data = NULL;
        bus[i].addr = 0;
        bus[i].len = 0;
    }

    // spawn 4 thread
    pthread_mutex_init(&mutex_bus, NULL);
    pthread_mutex_init(&mutex_finish, NULL);
    pthread_barrier_init(&barrier, NULL, 4);
    pthread_t core0, core1, core2, core3;
    int arg_core0 = 0, arg_core1 = 1, arg_core2 = 2, arg_core3 = 3;
    pthread_create(&core0, NULL, MESI_core, &arg_core0);
    pthread_create(&core1, NULL, MESI_core, &arg_core1);
    pthread_create(&core2, NULL, MESI_core, &arg_core2);
    pthread_create(&core3, NULL, MESI_core, &arg_core3);

    // simulation finished
    pthread_join(core0, NULL);
    pthread_join(core1, NULL);
    pthread_join(core2, NULL);
    pthread_join(core3, NULL);
    printf("> simulation finished.\n");
}

void* MESI_core(int* core_num_pointer)
{
    // init pointer
    int core_num = *core_num_pointer;
    printf("> core%d init...\n", core_num);
    FILE* input_file;
    int* state;
    uint32_t* tag;
    int* lru;
    if(core_num == 0){
        input_file = input_file_core0;
        state = state_core0;
        tag = tag_core0;
        lru = lru_core0;
    }
    else if(core_num == 1){
        input_file = input_file_core1;
        state = state_core1;
        tag = tag_core1;
        lru = lru_core1;
    }
    else if(core_num == 2){
        input_file = input_file_core2;
        state = state_core2;
        tag = tag_core2;
        lru = lru_core2;
    }
    else if(core_num == 3){
        input_file = input_file_core3;
        state = state_core3;
        tag = tag_core3;
        lru = lru_core3;
    }

    char log_name[100];
    sprintf(log_name, "output/core%d.log", core_num);
    FILE* log = fopen(log_name, "w+");

    pthread_barrier_wait(&barrier);

    // start simulation
    long long cycle = 0;
    long long next_inst = 0;
    char line[100];
    while(fgets(line, 100, input_file)){
        // get inst and addr
        char addr_s[11];
        uint32_t addr;
        int i;
        for(i = 2; line[i] != '\n'; i++){
            addr_s[i - 2] = line[i];
        }
        addr_s[i - 2] = '\0';
        sscanf(addr_s, "%x", &addr);

        // wait until inst being issued
        while(cycle != next_inst){
            // fprintf(log, "cycle %lld: executing\n", cycle);
            snoop_bus(core_num, state, tag);
            cycle++;
            if(core_num == 0 && cycle % 200000 == 0){
                printf("simulation cycle %lld\n", cycle);
            }
            pthread_barrier_wait(&barrier);
        }

        // load
        if(line[0] == '0'){
            uint32_t addr_tag = addr >> (offset_bits + index_bits);
            uint32_t addr_index = (addr & INDEX_MASK) >> offset_bits;
            int hit_flag = 0;
            for(int i = 0; i < assocaitivity; i++){
                if(addr_tag == tag[i * block_num + addr_index] && state[i * block_num + addr_index] != INVALID){
                    hit_flag = i + 1;
                    break;
                }
            }

            // if cache miss
            if(hit_flag == 0){
                fprintf(log, "cycle %lld: load, tag %x, index %x, cache miss\n", cycle, addr_tag, addr_index);
                // bus
                while(!bus_send(core_num, BUSRD, NULL, addr, 0)){
                    fprintf(log, "cycle %lld: load, waiting sending BUSRD\n", cycle);
                    snoop_bus(core_num, state, tag);
                    cycle++;
                    if(core_num == 0 && cycle % 200000 == 0){
                        printf("simulation cycle %lld\n", cycle);
                    }
                    pthread_barrier_wait(&barrier);
                }
                // wait for data
                int counter = 100;  // TODO
                while(counter != 0){
                    // fprintf(log, "cycle %lld: load, waiting memory data\n", cycle);
                    snoop_bus(core_num, state, tag);
                    cycle++;
                    if(core_num == 0 && cycle % 200000 == 0){
                        printf("simulation cycle %lld\n", cycle);
                    }
                    counter--;
                    pthread_barrier_wait(&barrier);
                }
                // refill
                int refill_way = 0; // TODO
                state[refill_way * block_num + addr_index] = SHARED; // TODO
                tag[refill_way * block_num + addr_index] = addr_tag;
                lru[refill_way * block_num + addr_index] = 0;
            }
            // if cache hit, update lru
            else{
                fprintf(log, "cycle %lld: load, tag %x, index %x, cache hit way %d\n", cycle, addr_tag, addr_index, hit_flag - 1);
                lru[(hit_flag - 1) * block_num + addr_index]++;
            }
            next_inst = cycle;
        }
        // store
        else if(line[0] == '1'){
            uint32_t addr_tag = addr >> (offset_bits + index_bits);
            uint32_t addr_index = (addr & INDEX_MASK) >> offset_bits;
            int hit_flag = 0;
            for(int i = 0; i < assocaitivity; i++){
                if(addr_tag == tag[i * block_num + addr_index] && state[i * block_num + addr_index] == MODIFIED){
                    hit_flag = 2 * i + 1;
                    break;
                }
                else if(addr_tag == tag[i * block_num + addr_index] && state[i * block_num + addr_index] != INVALID){
                    hit_flag = 2 * i + 2;
                    break;
                }
            }
                  
            // if cache miss
            if(hit_flag == 0){
                fprintf(log, "cycle %lld: store, tag %x, index %x, cache miss\n", cycle, addr_tag, addr_index);
                // bus
                while(!bus_send(core_num, BUSRDX, NULL, addr, 0)){
                    fprintf(log, "cycle %lld: store, waiting sending BUSRD\n", cycle);
                    snoop_bus(core_num, state, tag);
                    cycle++;
                    if(core_num == 0 && cycle % 200000 == 0){
                        printf("simulation cycle %lld\n", cycle);
                    }
                    pthread_barrier_wait(&barrier);
                }
                // wait for data
                int counter = 100; // TODO
                while(counter != 0){
                    // fprintf(log, "cycle %lld: store, waiting memory data\n", cycle);
                    snoop_bus(core_num, state, tag);
                    cycle++;
                    if(core_num == 0 && cycle % 200000 == 0){
                        printf("simulation cycle %lld\n", cycle);
                    }
                    counter--;
                    pthread_barrier_wait(&barrier);
                }
                // refill
                int refill_way = 0; // TODO
                state[refill_way * block_num + addr_index] = MODIFIED; 
                tag[refill_way * block_num + addr_index] = addr_tag;
            }
            // not in M state
            else if(hit_flag % 2 == 0){
                fprintf(log, "cycle %lld: store, tag %x, index %x, cache hit way %d not in M state\n", cycle, addr_tag, addr_index, (hit_flag - 2) / 2);
                // busrdx
                while(!bus_send(core_num, BUSRDX, NULL, addr, 0)){
                    fprintf(log, "cycle %lld: store, waiting memory data\n", cycle);
                    snoop_bus(core_num, state, tag);
                    cycle++;
                    if(core_num == 0 && cycle % 200000 == 0){
                        printf("simulation cycle %lld\n", cycle);
                    }
                    pthread_barrier_wait(&barrier);
                }
                state[((hit_flag - 2) / 2) * block_num + addr_index] = MODIFIED; 
                lru[((hit_flag - 2) / 2) * block_num + addr_index]++;
            }
            // if cache hit, update lru
            else{
                fprintf(log, "cycle %lld: store, tag %x, index %x, cache hit way %d\n", cycle, addr_tag, addr_index, (hit_flag - 1) / 2);
                lru[((hit_flag - 1) / 2) * block_num + addr_index]++;
            }
            next_inst = cycle;
        }
        else{
            next_inst += addr;
        }
    }
    
    printf("> core%d finish at cycle %d\n", core_num, cycle);
    pthread_mutex_lock(&mutex_finish);
    finish++;
    pthread_barrier_wait(&barrier); // TODO
    pthread_mutex_unlock(&mutex_finish);
    while(finish != 4){
        pthread_barrier_wait(&barrier);
    }
    fclose(log);
    pthread_exit(NULL);
}

void snoop_bus(int core_num, int* state, uint32_t* tag)
{
    pthread_mutex_lock(&mutex_bus);
    for(int i = 0; i < 4; i++){
        if(bus[i].sender != core_num && bus[i].busy){
            uint32_t bus_tag = bus[i].addr >> (offset_bits + index_bits);
            uint32_t bus_index = (bus[i].addr & INDEX_MASK) >> offset_bits;
            int hit_flag = 0;
            if(bus_tag == tag[bus_index] && state[bus_index] != INVALID){
                hit_flag = 1;
            }
            if(bus_tag == tag[block_num + bus_index] && state[block_num + bus_index] != INVALID){
                hit_flag = 2;
            }
            if(!hit_flag){
                bus[i].busy--;
                continue;
            }

            if(bus[i].tran = BUSRD){
                if(state[(hit_flag - 1) * block_num + bus_index] == MODIFIED){
                    // TODO
                }
                else if(state[(hit_flag - 1) * block_num + bus_index] == EXCLUSIVE){
                    state[(hit_flag - 1) * block_num + bus_index] = SHARED;
                }
            }
            else if(bus[i].tran = BUSRDX){
                if(state[(hit_flag - 1) * block_num + bus_index] == MODIFIED){
                    // TODO
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