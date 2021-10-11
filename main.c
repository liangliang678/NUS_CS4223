#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "coherence.h"

int protocol;
FILE* input_file_core0;
FILE* input_file_core1;
FILE* input_file_core2;
FILE* input_file_core3;
int cache_size = 4096;
int assocaitivity = 2;
int block_size = 32;

int main(int argc, char** argv)
{
    if(argc <= 3){
        printf("too few parameters!\n");
        printf("usage: ./coherence protocol input_file (cache_size associativity block_size).\n");
        exit(1);
    }
    
    // protocol
    if(!strcmp(argv[1], "MESI")){
        protocol = MESI;
        printf("> set protocol: MESI\n");
    }
    else if(!strcmp(argv[1], "Dragon")){
        protocol = DRAGON;
        printf("> set protocol: Dragon\n");
    }
    else{
        printf("unknown protocal!\n");
        exit(1);
    }

    // input_file
    char dir_name[50] = {0};
    char file_name_core0[100];
    char file_name_core1[100];
    char file_name_core2[100];
    char file_name_core3[100];
    strcat(dir_name, argv[2]);
    strcat(dir_name, "_four/");
    sprintf(file_name_core0, "%s%s_0.data", dir_name, argv[2]);
    sprintf(file_name_core1, "%s%s_1.data", dir_name, argv[2]);
    sprintf(file_name_core2, "%s%s_2.data", dir_name, argv[2]);
    sprintf(file_name_core3, "%s%s_3.data", dir_name, argv[2]);
    printf("> opening %s*.\n", dir_name);
    input_file_core0 = fopen(file_name_core0, "r");
    input_file_core1 = fopen(file_name_core1, "r");
    input_file_core2 = fopen(file_name_core2, "r");
    input_file_core3 = fopen(file_name_core3, "r");
    if(!input_file_core0 || !input_file_core1 || !input_file_core2 || !input_file_core3){
        printf("failed when opening input file!\n");
        exit(1);
    }
    else{
        printf("> successfully opened 4 input files.\n");
    }

    // cache size
    if(argc >= 4){
        cache_size = atoi(argv[4]);
    }

    // assocaitivity
    if(argc >= 5){
        assocaitivity = atoi(argv[5]);
    }

    // block_size
    if(argc >= 6){
        block_size = atoi(argv[6]);
    }

    if(argc > 6){
        printf("too many parameters!\n");
        exit(1);
    }

    printf("> cache_size: %dBytes.\n", cache_size);
    printf("> assocaitivity: %d.\n", assocaitivity);
    printf("> block_size: %dBytes.\n", block_size);
}