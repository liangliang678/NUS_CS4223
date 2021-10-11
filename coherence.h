#ifndef COHERENCE_H
#define COHERENCE_H

#define MESI 1
#define DRAGON 2

#define MODIFIED 1
#define EXCLUSIVE 2
#define SHARED 3
#define INVALID 4

extern int protocol;
extern FILE* input_file_core0;
extern FILE* input_file_core1;
extern FILE* input_file_core2;
extern FILE* input_file_core3;
extern int cache_size;
extern int assocaitivity;
extern int block_size;
extern int block_num;

extern void simulate_MESI();
extern void* MESI_core(int* core_num_pointer);
extern void simulate_Dragon();

#endif
