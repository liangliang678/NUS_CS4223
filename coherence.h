#ifndef COHERENCE_H
#define COHERENCE_H

#define MESI 1
#define DRAGON 2

extern int protocol;
extern FILE* input_file_core0;
extern FILE* input_file_core1;
extern FILE* input_file_core2;
extern FILE* input_file_core3;
extern int cache_size;
extern int assocaitivity;
extern int block_size;

#endif