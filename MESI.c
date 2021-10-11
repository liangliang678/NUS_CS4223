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
    lru_core0 = (int*)malloc(block_num * sizeof(int));
    lru_core1 = (int*)malloc(block_num * sizeof(int));
    lru_core2 = (int*)malloc(block_num * sizeof(int));
    lru_core3 = (int*)malloc(block_num * sizeof(int));

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
        }
    }

    // spawn 4 thread
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
    pthread_barrier_wait(&barrier);

    char line[100];
    while(fgets(line, 100, input_file)){
        // TODO
    }
    
    printf("> core%d finish...\n", core_num);
    pthread_exit(NULL);
}
