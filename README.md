# CS4223 Project Report

## Usage

create a folder called `output` in this repo, download blackscholes_four, bodytrack_four, fluidanimate_four.

The command line should be like
```shell
make
coherence protocol input_file cache_size associativity block_size
make clean 
```
where coherence is the executable file name and input parameters are
- `protocol` is MESI or Dragon or my
- `input_file` is the input benchmark name (e.g., bodytrack)
- `cache_size`: cache size in bytes
- `associativity`: associativity of the cache
- `block_size`: block size in bytes

You can refer to `core*.log` as well as command line output for simulation result.

## Files

| file name | usage |
| -- | -- |
| blackscholes_four | testbench |
| bodytrack_four | testbench |
| fluidanimate_four | testbench | 
| small_four | testbench created by us |
| output | folder for log files |
| .gitignore | ignore file |
| coherence.h | header file |
| Makefile | Makefile |
| bus.c | src code of bus |
| main.c | src code of input parser | 
| MESI.c | src code of MESI protocol |
| Dragon.c | src code of Dragon protocol |
| my.c | src code of modified Dragon protocol |
| report.md | project report |
| result.md | command line output of each testbench |
