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

int finish_dragon = 0;
pthread_mutex_t mutex_finish_dragon;
long long max_cycle;

int check_share_dragon(uint32_t tag, uint32_t index)
{
    int ret = 0;
    for(int i = 0; i < associativity; i++){
        if(state_core0[i * block_num + index] != INVALID && tag_core0[i * block_num + index] == tag){
            ret++;
        }
        if(state_core1[i * block_num + index] != INVALID && tag_core1[i * block_num + index] == tag){
            ret++;
        }
        if(state_core2[i * block_num + index] != INVALID && tag_core2[i * block_num + index] == tag){
            ret++;
        }
        if(state_core3[i * block_num + index] != INVALID && tag_core3[i * block_num + index] == tag){
            ret++;
        }
    }
    return ret;
}

void simulate_Dragon(){
    printf("> simulation init...\n");
    state_core0 = (int*)malloc(associativity * block_num * sizeof(int));
    state_core1 = (int*)malloc(associativity * block_num * sizeof(int));
    state_core2 = (int*)malloc(associativity * block_num * sizeof(int));
    state_core3 = (int*)malloc(associativity * block_num * sizeof(int));
    tag_core0 = (uint32_t*)malloc(associativity * block_num * sizeof(uint32_t));
    tag_core1 = (uint32_t*)malloc(associativity * block_num * sizeof(uint32_t));
    tag_core2 = (uint32_t*)malloc(associativity * block_num * sizeof(uint32_t));
    tag_core3 = (uint32_t*)malloc(associativity * block_num * sizeof(uint32_t));
    lru_core0 = (int*)malloc(associativity * block_num * sizeof(int));
    lru_core1 = (int*)malloc(associativity * block_num * sizeof(int));
    lru_core2 = (int*)malloc(associativity * block_num * sizeof(int));
    lru_core3 = (int*)malloc(associativity * block_num * sizeof(int));

    for(int i = 0; i < associativity; i++){
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
        bus[i].recv[0] = 0;
        bus[i].recv[1] = 0;
        bus[i].recv[2] = 0;
        bus[i].recv[3] = 0;
        bus[i].tran = 0;
        bus[i].addr = 0;
        bus[i].len = 0;
    }

    pthread_mutex_init(&mutex_bus, NULL);
    pthread_mutex_init(&mutex_finish_dragon, NULL);
    pthread_barrier_init(&barrier, NULL, 4);
    pthread_t core0, core1, core2, core3;
    int arg_core0 = 0, arg_core1 = 1, arg_core2 = 2, arg_core3 = 3;
    pthread_create(&core0, NULL, DRAGON_core, &arg_core0);
    pthread_create(&core1, NULL, DRAGON_core, &arg_core1);
    pthread_create(&core2, NULL, DRAGON_core, &arg_core2);
    pthread_create(&core3, NULL, DRAGON_core, &arg_core3);

    // simulation finish_dragoned
    pthread_join(core0, NULL);
    pthread_join(core1, NULL);
    pthread_join(core2, NULL);
    pthread_join(core3, NULL);
    printf("> simulation finish_dragoned at cycle %lld.\n", max_cycle);
    printf("> bus_wb: %lld, bus_inv: %lld\n", bus_wb, bus_inv);
}

void* DRAGON_core(void* core_num_pointer)
{
    int core_num = *(int*)core_num_pointer;
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
    long long compute_cycle = 0;
    long long load_inst_num = 0;
    long long store_inst_num = 0;
    long long cache_hit = 0;
    long long cache_miss = 0;
    long long bus_idle = 0;
    long long mem_idle = 0;
    long long private_acc = 0;
    long long shared_acc = 0;

    char line[100];
    long line_num = 0;
    while(fgets(line, 100, input_file)){
        line_num++;
        if(line_num % 100000 == 0){
            printf("core%d input file line %ld\n", core_num, line_num);
        }
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
        while(cycle < next_inst){
            int wb = snoop_bus_dragon(core_num, state, tag, &cycle);
            if(!wb){
                cycle++;
                pthread_barrier_wait(&barrier);
            }
        }

        // load
        if(line[0] == '0'){
            load_inst_num++;
            uint32_t addr_tag = addr >> (offset_bits + index_bits);
            uint32_t addr_index = (addr & INDEX_MASK) >> offset_bits;
            int hit_flag = 0;
            for(int i = 0; i < associativity; i++){
                if(addr_tag == tag[i * block_num + addr_index] && state[i * block_num + addr_index] != INVALID){
                    hit_flag = i + 1;
                    break;
                }
            }

            // if cache miss
            if(hit_flag == 0){
                // Read miss: 
                cache_miss++;
                fprintf(log, "cycle %lld: load, tag %x, index %x, cache miss\n", cycle, addr_tag, addr_index);
                // bus
                fprintf(log, "cycle %lld: load, start waiting to send BUSRD\n", cycle);
                while(!bus_send(core_num, BUSRD, addr, 0)){
                    int wb = snoop_bus_dragon(core_num, state, tag, &cycle);
                    if(!wb){
                        cycle++;
                        bus_idle++;
                        pthread_barrier_wait(&barrier);
                    }
                }
                fprintf(log, "cycle %lld: load, sent BUSRD, start waiting BUSRD ack\n", cycle);
                // wait for bus ack
                while(!bus_recv(core_num)){
                    int wb = snoop_bus_dragon(core_num, state, tag, &cycle);
                    if(!wb){
                        cycle++;
                        bus_idle++;
                        pthread_barrier_wait(&barrier);
                    }
                }
                fprintf(log, "cycle %lld: load, BUSRD ack\n", cycle);
                // replace
                int refill_way = 0;
                for(int i = 0; i < associativity; i++){
                    if(lru[i * block_num + addr_index] < lru[refill_way * block_num + addr_index]){
                        refill_way = i;
                    }
                }

                // wait for data
                fprintf(log, "cycle %lld: load, start waiting data to refill\n", cycle);
                int counter = 100;  
                while(counter != 0){
                    int wb = snoop_bus_dragon(core_num, state, tag, NULL);
                    if(!wb){
                        cycle++;
                        mem_idle++;
                        counter--;
                        pthread_barrier_wait(&barrier);
                    }
                }
                // refill
                // Need to check whether others share or not
                state[refill_way * block_num + addr_index] = (check_share_dragon(addr_tag, addr_index) == 0) ? E : Sc;
                tag[refill_way * block_num + addr_index] = addr_tag;
                lru[refill_way * block_num + addr_index] = 0;
                bus_cancle(core_num); 
                if(state[refill_way * block_num + addr_index] == Sm || state[refill_way * block_num + addr_index] == Sc){
                    shared_acc++;
                }
                else{
                    private_acc++;
                }
            }
            // if cache hit, update lru
            else {
                cache_hit++;
                fprintf(log, "cycle %lld: load, tag %x, index %x, cache hit way %d\n", cycle, addr_tag, addr_index, hit_flag - 1);
                lru[(hit_flag - 1) * block_num + addr_index]++;
                if(state[(hit_flag - 1) * block_num + addr_index] == Sm || state[(hit_flag - 1) * block_num + addr_index] == Sc){
                    shared_acc++;
                }
                else{
                    private_acc++;
                }
            }
            next_inst = cycle + 1;
        }
        // store
        else if(line[0] == '1'){
            store_inst_num++;
            uint32_t addr_tag = addr >> (offset_bits + index_bits);
            uint32_t addr_index = (addr & INDEX_MASK) >> offset_bits;
            int hit_flag = 0;
            for(int i = 0; i < associativity; i++){
                if(addr_tag == tag[i * block_num + addr_index] && state[i * block_num + addr_index] != INVALID){
                    hit_flag = i + 1;
                    break;
                }
            }
                  
            // if cache miss 
            if(hit_flag == 0){
                cache_miss++;
                fprintf(log, "cycle %lld: store, tag %x, index %x, cache miss\n", cycle, addr_tag, addr_index);
                // bus
                fprintf(log, "cycle %lld: store, start waiting to send BUSRD\n", cycle);
                while(!bus_send(core_num, BUSRD, addr, 0)){
                    int wb = snoop_bus_dragon(core_num, state, tag, &cycle);
                    if(!wb){
                        cycle++;
                        bus_idle++;
                        pthread_barrier_wait(&barrier);
                    }
                }
                fprintf(log, "cycle %lld: store, sent BUSRD, start waiting BUSRD ack\n", cycle);
                // wait for bus ack
                while(!bus_recv(core_num)){
                    int wb = snoop_bus_dragon(core_num, state, tag, &cycle);
                    if(!wb){
                        cycle++;
                        bus_idle++;
                        pthread_barrier_wait(&barrier);
                    }
                }
                // wait for data
                fprintf(log, "cycle %lld: load, start waiting data to refill\n", cycle);
                int counter = 100;  
                while(counter != 0){
                    int wb = snoop_bus_dragon(core_num, state, tag, NULL);
                    if(!wb){
                        cycle++;
                        mem_idle++;
                        counter--;
                        pthread_barrier_wait(&barrier);
                    }
                }
                bus_cancle(core_num);
                fprintf(log, "cycle %lld: store, start waiting to send BUSUPD\n", cycle);
                while(!bus_send(core_num, BUSUPD, addr, 4)){
                    int wb = snoop_bus_dragon(core_num, state, tag, &cycle);
                    if(!wb){
                        cycle++;
                        bus_idle++;
                        pthread_barrier_wait(&barrier);
                    }
                }
                fprintf(log, "cycle %lld: store, sent BUSUPD, start waiting BUSUPD ack\n", cycle);
                // wait for bus ack
                while(!bus_recv(core_num)){
                    int wb = snoop_bus_dragon(core_num, state, tag, &cycle);
                    if(!wb){
                        cycle++;
                        bus_idle++;
                        pthread_barrier_wait(&barrier);
                    }
                }
                fprintf(log, "cycle %lld: store, BUSUPD ack\n", cycle);
                // replace
                int refill_way = 0;
                for(int i = 0; i < associativity; i++){
                    if(lru[i * block_num + addr_index] < lru[refill_way * block_num + addr_index]){
                        refill_way = i;
                    }
                }
                // refill
                state[refill_way * block_num + addr_index] = (check_share_dragon(addr_tag, addr_index) == 0) ? M : Sm; 
                tag[refill_way * block_num + addr_index] = addr_tag;
                lru[refill_way * block_num + addr_index] = 0;
                bus_cancle(core_num);
            }
            else if(state[(hit_flag - 1) * block_num + addr_index] == M){
                // Nothing happens
                cache_hit++;
                fprintf(log, "cycle %lld: store, tag %x, index %x, cache hit way %d in M state\n", cycle, addr_tag, addr_index, hit_flag - 1);
                state[(hit_flag - 1) * block_num + addr_index] = M; 
                lru[(hit_flag - 1) * block_num + addr_index]++;
            }
            else if(state[(hit_flag - 1) * block_num + addr_index] == Sm){
                cache_hit++;
                fprintf(log, "cycle %lld: store, tag %x, index %x, cache hit way %d in Sm state\n", cycle, addr_tag, addr_index, hit_flag - 1);
                if (check_share_dragon(addr_tag, addr_index) == 0){
                    // shared line is not asserted
                    state[(hit_flag - 1) * block_num + addr_index] = M; 
                    lru[(hit_flag - 1) * block_num + addr_index]++;
                }else{
                    // shared line is asserted
                    fprintf(log, "cycle %lld: store, start waiting to send BUSUPD\n", cycle);
                    while(!bus_send(core_num, BUSUPD, addr, 4)){
                        int wb = snoop_bus_dragon(core_num, state, tag, &cycle);
                        if(!wb){
                            cycle++;
                            bus_idle++;
                            pthread_barrier_wait(&barrier);
                        }
                    }
                    fprintf(log, "cycle %lld: store, sent BUSUPD, start waiting BUSUPD ack\n", cycle);
                    // wait for bus ack
                    while(!bus_recv(core_num)){
                        int wb = snoop_bus_dragon(core_num, state, tag, &cycle);
                        if(!wb){
                            cycle++;
                            bus_idle++;
                            pthread_barrier_wait(&barrier);
                        }
                    }
                    fprintf(log, "cycle %lld: store, BUSUPD ack\n", cycle);
                    state[(hit_flag - 1) * block_num + addr_index] = Sm; 
                    lru[(hit_flag - 1) * block_num + addr_index]++;
                    bus_cancle(core_num);
                }
            }
            // if cache hit, update lru
            else if(state[(hit_flag - 1) * block_num + addr_index] == Sc){
                cache_hit++;
                fprintf(log, "cycle %lld: store, tag %x, index %x, cache hit way %d\n", cycle, addr_tag, addr_index, hit_flag - 1);
                if (check_share_dragon(addr_tag, addr_index) == 0){
                    // shared line is not asserted
                    state[(hit_flag - 1) * block_num + addr_index] = M; 
                    lru[(hit_flag - 1) * block_num + addr_index]++;
                }else{
                    // shared line is asserted
                    fprintf(log, "cycle %lld: store, start waiting to send BUSUPD\n", cycle);
                    while(!bus_send(core_num, BUSUPD, addr, 4)){
                        int wb = snoop_bus_dragon(core_num, state, tag, &cycle);
                        if(!wb){
                            cycle++;
                            bus_idle++;
                            pthread_barrier_wait(&barrier);
                        }
                    }
                    fprintf(log, "cycle %lld: store, sent BUSUPD, start waiting BUSUPD ack\n", cycle);
                    // wait for bus ack
                    while(!bus_recv(core_num)){
                        int wb = snoop_bus_dragon(core_num, state, tag, &cycle);
                        if(!wb){
                            cycle++;
                            bus_idle++;
                            pthread_barrier_wait(&barrier);
                        }
                    }
                    fprintf(log, "cycle %lld: store, BUSUPD ack\n", cycle);
                    state[(hit_flag - 1) * block_num + addr_index] = Sm; 
                    lru[(hit_flag - 1) * block_num + addr_index]++;
                    bus_cancle(core_num);
                }
            }
            else{
                cache_hit++;
                fprintf(log, "cycle %lld: store, tag %x, index %x, cache hit way %d\n", cycle, addr_tag, addr_index, hit_flag - 1);
                state[(hit_flag - 1) * block_num + addr_index] = M; 
                lru[(hit_flag - 1) * block_num + addr_index]++;
            }
            next_inst = cycle + 1;
        }
        else{
            next_inst += addr;
            compute_cycle += addr;
        }
    }
    
    while(cycle < next_inst){
        int wb = snoop_bus_dragon(core_num, state, tag, &cycle);
        if(!wb){
            cycle++;
            pthread_barrier_wait(&barrier);
        }
    }
    
    printf("> core%d finish_dragon at cycle %lld\n", core_num, cycle);
    printf("> core%d compute_cycle: %lld, bus_idle: %lld, mem_idle: %lld\n", core_num, compute_cycle, bus_idle, mem_idle);
    printf("> core%d load_inst_num: %lld, store_inst_num: %lld\n", core_num, load_inst_num, store_inst_num);
    printf("> core%d private_acc: %lld, shared_acc: %lld\n", core_num, private_acc, shared_acc);
    printf("> core%d cache_hit: %lld, cache_miss: %lld, hit_rate: %lf\n", core_num, cache_hit, cache_miss, cache_hit * 1.0 / (cache_hit + cache_miss));

    pthread_mutex_lock(&mutex_finish_dragon);
    finish_dragon++;
    max_cycle = cycle;
    pthread_mutex_unlock(&mutex_finish_dragon);

    do{
        int wb = snoop_bus_dragon(core_num, state, tag, &cycle);
        if(!wb){
            cycle++;
            pthread_barrier_wait(&barrier);
        }
        
    } while(finish_dragon != 4);

    fclose(log);
    pthread_exit(NULL);
}
