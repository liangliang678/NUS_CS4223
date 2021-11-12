./coherence MESI blackscholes 8192 2 32

```
> set protocol: MESI
> opening blackscholes_four/*
> successfully opened 4 input files
> cache_size: 8192 Bytes
> associativity: 2
> block_size: 32 Bytes
> simulation init...
> core0 init...
> core1 init...
> core2 init...
> core3 init...

> core1 finish at cycle 15170174
> core1 compute_cycle: 10383276, bus_idle: 37008, mem_idle: 2260200
> core1 load_inst_num: 1485857, store_inst_num: 1004611
> core1 private_acc: 1013516, shared_acc: 472341
> core1 cache_hit: 2468875, cache_miss: 21593, hit_rate: 0.991330

> core3 finish at cycle 15210592
> core3 compute_cycle: 10394904, bus_idle: 37853, mem_idle: 2276100
> core3 load_inst_num: 1493736, store_inst_num: 1009391
> core3 private_acc: 1018315, shared_acc: 475421
> core3 cache_hit: 2481258, cache_miss: 21869, hit_rate: 0.991263

> core0 finish at cycle 15279788
> core0 compute_cycle: 10430314, bus_idle: 38056, mem_idle: 2314600
> core0 load_inst_num: 1489888, store_inst_num: 1007461
> core0 private_acc: 1015970, shared_acc: 473918
> core0 cache_hit: 2475364, cache_miss: 21985, hit_rate: 0.991197

> core2 finish at cycle 19862713
> core2 compute_cycle: 10430338, bus_idle: 105810, mem_idle: 6818300
> core2 load_inst_num: 1492629, store_inst_num: 1016428
> core2 private_acc: 1021312, shared_acc: 471317
> core2 cache_hit: 2444858, cache_miss: 64199, hit_rate: 0.974413

> simulation finished at cycle 19862713.
> bus_rd: 129646, bus_wb: 7047, bus_inv: 359
```

./coherence MESI bodytrack 8192 2 32

```
> set protocol: MESI
> opening bodytrack_four/*
> successfully opened 4 input files
> cache_size: 8192 Bytes
> associativity: 2
> block_size: 32 Bytes
> simulation init...
> core1 init...
> core2 init...
> core3 init...
> core0 init...

> core2 finish at cycle 18797112
> core2 compute_cycle: 17556877, bus_idle: 24583, mem_idle: 1100700
> core2 load_inst_num: 74523, store_inst_num: 43175
> core2 private_acc: 50528, shared_acc: 23995
> core2 cache_hit: 109214, cache_miss: 8484, hit_rate: 0.927917

> core1 finish at cycle 42455875
> core1 compute_cycle: 17120545, bus_idle: 302008, mem_idle: 21747500
> core1 load_inst_num: 2388005, store_inst_num: 899247
> core1 private_acc: 1653393, shared_acc: 734612
> core1 cache_hit: 3096541, cache_miss: 190711, hit_rate: 0.941985

> core0 finish at cycle 42643380
> core0 compute_cycle: 17729254, bus_idle: 335647, mem_idle: 21314100
> core0 load_inst_num: 2380720, store_inst_num: 889412
> core0 private_acc: 1606451, shared_acc: 774269
> core0 cache_hit: 3083048, cache_miss: 187084, hit_rate: 0.942790

> core3 finish at cycle 43311229
> core3 compute_cycle: 17140113, bus_idle: 344422, mem_idle: 22508100
> core3 load_inst_num: 2416052, store_inst_num: 908867
> core3 private_acc: 1605903, shared_acc: 810149
> core3 cache_hit: 3128212, cache_miss: 196707, hit_rate: 0.940839

> simulation finished at cycle 43311229.
> bus_rd: 582986, bus_wb: 83721, bus_inv: 1445
```

./coherence MESI fluidanimate 8192 2 32

```
> set protocol: MESI
> opening fluidanimate_four/*
> successfully opened 4 input files
> cache_size: 8192 Bytes
> associativity: 2
> block_size: 32 Bytes
> simulation init...
> core0 init...
> core1 init...
> core2 init...
> core3 init...

> core3 finish at cycle 44969397
> core3 compute_cycle: 11301515, bus_idle: 394105, mem_idle: 26156000
> core3 load_inst_num: 1832174, store_inst_num: 579291
> core3 private_acc: 1395268, shared_acc: 436906
> core3 cache_hit: 2244861, cache_miss: 166604, hit_rate: 0.930912

> core1 finish at cycle 40800710
> core1 compute_cycle: 11290799, bus_idle: 376560, mem_idle: 26732100
> core1 load_inst_num: 1821846, store_inst_num: 585998
> core1 private_acc: 1382913, shared_acc: 438933
> core1 cache_hit: 2238410, cache_miss: 169434, hit_rate: 0.929632

> core0 finish at cycle 42474796
> core0 compute_cycle: 11337782, bus_idle: 318837, mem_idle: 28243700
> core0 load_inst_num: 1832392, store_inst_num: 744111
> core0 private_acc: 1395169, shared_acc: 437223
> core0 cache_hit: 2400363, cache_miss: 176140, hit_rate: 0.931636

> core2 finish at cycle 44969397
> core2 compute_cycle: 11337671, bus_idle: 350732, mem_idle: 30679100
> core2 load_inst_num: 1838008, store_inst_num: 766181
> core2 private_acc: 1390413, shared_acc: 447595
> core2 cache_hit: 2411095, cache_miss: 193094, hit_rate: 0.925853

> simulation finished at cycle 44969397.
> bus_rd: 705272, bus_wb: 412842, bus_inv: 2219
```

./coherence Dragon blackscholes 8192 2 32

```
> set protocol: Dragon
> opening blackscholes_four/*
> successfully opened 4 input files
> cache_size: 8192 Bytes
> associativity: 2
> block_size: 32 Bytes
> simulation init...
> core0 init...
> core2 init...
> core1 init...
> core3 init...

> core1 finish_dragon at cycle 17004743
> core1 compute_cycle: 10383276, bus_idle: 30032, mem_idle: 4101672
> core1 load_inst_num: 1485857, store_inst_num: 1004611
> core1 private_load: 65806, shared_load: 1420051
> core1 private_store: 45388, shared_store: 959223
> core1 cache_hit: 2469406, cache_miss: 21062, hit_rate: 0.991543

> core3 finish_dragon at cycle 17176776
> core3 compute_cycle: 10394904, bus_idle: 37537, mem_idle: 4242432
> core3 load_inst_num: 1493736, store_inst_num: 1009391
> core3 private_load: 38083, shared_load: 1455653
> core3 private_store: 18318, shared_store: 991073
> core3 cache_hit: 2481388, cache_miss: 21739, hit_rate: 0.991315

> core0 finish_dragon at cycle 17224201
> core0 compute_cycle: 10430314, bus_idle: 34790, mem_idle: 4262392
> core0 load_inst_num: 1489888, store_inst_num: 1007461
> core0 private_load: 42773, shared_load: 1447115
> core0 private_store: 23015, shared_store: 984446
> core0 cache_hit: 2475518, cache_miss: 21831, hit_rate: 0.991258

> core2 finish_dragon at cycle 19852434
> core2 compute_cycle: 10430338, bus_idle: 101087, mem_idle: 6812068
> core2 load_inst_num: 1492629, store_inst_num: 1016428
> core2 private_load: 1019811, shared_load: 472818
> core2 private_store: 1015664, shared_store: 764
> core2 cache_hit: 2444902, cache_miss: 64155, hit_rate: 0.974431

> simulation finished at cycle 19852434.
> bus_rd: 128787, bus_wb: 6574, bus_upd: 17676
```

/coherence Dragon bodytrack 8192 2 32

```
> set protocol: Dragon
> opening bodytrack_four/*
> successfully opened 4 input files
> cache_size: 8192 Bytes
> associativity: 2
> block_size: 32 Bytes
> simulation init...
> core0 init...
> core1 init...
> core2 init...
> core3 init...

> core2 finish_dragon at cycle 18794839
> core2 compute_cycle: 17556877, bus_idle: 20421, mem_idle: 1109776
> core2 load_inst_num: 74523, store_inst_num: 43175
> core2 private_load: 50396, shared_load: 24127
> core2 private_store: 42726, shared_store: 449
> core2 cache_hit: 109240, cache_miss: 8458, hit_rate: 0.928138

> core1 finish_dragon at cycle 42483009
> core1 compute_cycle: 17120545, bus_idle: 298860, mem_idle: 21778040
> core1 load_inst_num: 2388005, store_inst_num: 899247
> core1 private_load: 1613696, shared_load: 774309
> core1 private_store: 898358, shared_store: 889
> core1 cache_hit: 3096619, cache_miss: 190633, hit_rate: 0.942008

> core0 finish_dragon at cycle 42577499
> core0 compute_cycle: 17729254, bus_idle: 304179, mem_idle: 21278644
> core0 load_inst_num: 2380720, store_inst_num: 889412
> core0 private_load: 1598069, shared_load: 782651
> core0 private_store: 887397, shared_store: 2015
> core0 cache_hit: 3083485, cache_miss: 186647, hit_rate: 0.942924

> core3 finish_dragon at cycle 43185964
> core3 compute_cycle: 17140113, bus_idle: 321453, mem_idle: 22405126
> core3 load_inst_num: 2416052, store_inst_num: 908867
> core3 private_load: 1588032, shared_load: 828020
> core3 private_store: 906584, shared_store: 2283
> core3 cache_hit: 3129465, cache_miss: 195454, hit_rate: 0.941215

> simulation finished at cycle 43185964.
> bus_rd: 581192, bus_wb: 83214, bus_upd: 24596
```

./coherence Dragon fluidanimate 8192 2 32

```
> set protocol: Dragon
> opening fluidanimate_four/*
> successfully opened 4 input files
> cache_size: 8192 Bytes
> associativity: 2
> block_size: 32 Bytes
> simulation init...
> core0 init...
> core1 init...
> core2 init...
> core3 init...

> core3 finish_dragon at cycle 39895892
> core3 compute_cycle: 11301515, bus_idle: 400881, mem_idle: 25784620
> core3 load_inst_num: 1832174, store_inst_num: 579291
> core3 private_load: 1374774, shared_load: 457400
> core3 private_store: 564682, shared_store: 14609
> core3 cache_hit: 2248296, cache_miss: 163169, hit_rate: 0.932336

> core1 finish_dragon at cycle 40390818
> core1 compute_cycle: 11290799, bus_idle: 439849, mem_idle: 26254946
> core1 load_inst_num: 1821846, store_inst_num: 585998
> core1 private_load: 1365192, shared_load: 456654
> core1 private_store: 574125, shared_store: 11873
> core1 cache_hit: 2243014, cache_miss: 164830, hit_rate: 0.931545

> core0 finish_dragon at cycle 42947535
> core0 compute_cycle: 11337782, bus_idle: 406445, mem_idle: 28627718
> core0 load_inst_num: 1832392, store_inst_num: 744111
> core0 private_load: 1386113, shared_load: 446279
> core0 private_store: 736207, shared_store: 7904
> core0 cache_hit: 2397203, cache_miss: 179300, hit_rate: 0.930410

> core2 finish_dragon at cycle 45123307
> core2 compute_cycle: 11337671, bus_idle: 349541, mem_idle: 30835138
> core2 load_inst_num: 1838008, store_inst_num: 766181
> core2 private_load: 1389175, shared_load: 448833
> core2 private_store: 764315, shared_store: 1866
> core2 cache_hit: 2411779, cache_miss: 192410, hit_rate: 0.926115

> simulation finished at cycle 45123307.
> bus_rd: 699709, bus_wb: 407855, bus_upd: 49660
```

./coherence my blackscholes 8192 2 32

```
> set protocol: my
> opening blackscholes_four/*
> successfully opened 4 input files
> cache_size: 8192 Bytes
> associativity: 2
> block_size: 32 Bytes
> simulation init...
> core0 init...
> core2 init...
> core3 init...
> core1 init...

> core1 finish_my at cycle 17002941
> core1 compute_cycle: 10383276, bus_idle: 28870, mem_idle: 4100998
> core1 load_inst_num: 1485857, store_inst_num: 1004611
> core1 private_load: 65446, shared_load: 1420411
> core1 private_store: 45001, shared_store: 959610
> core1 cache_hit: 2469406, cache_miss: 21062, hit_rate: 0.991543

> core3 finish_my at cycle 17169329
> core3 compute_cycle: 10394904, bus_idle: 34002, mem_idle: 4238394
> core3 load_inst_num: 1493736, store_inst_num: 1009391
> core3 private_load: 38160, shared_load: 1455576
> core3 private_store: 18389, shared_store: 991002
> core3 cache_hit: 2481388, cache_miss: 21739, hit_rate: 0.991315

> core0 finish_my at cycle 17219670
> core0 compute_cycle: 10430314, bus_idle: 32472, mem_idle: 4259860
> core0 load_inst_num: 1489888, store_inst_num: 1007461
> core0 private_load: 42669, shared_load: 1447219
> core0 private_store: 22925, shared_store: 984536
> core0 cache_hit: 2475518, cache_miss: 21831, hit_rate: 0.991258

> core2 finish_my at cycle 19843369
> core2 compute_cycle: 10430338, bus_idle: 97845, mem_idle: 6806228
> core2 load_inst_num: 1492629, store_inst_num: 1016428
> core2 private_load: 1019811, shared_load: 472818
> core2 private_store: 1015664, shared_store: 764
> core2 cache_hit: 2444902, cache_miss: 64155, hit_rate: 0.974431

> simulation finished at cycle 19843369.
> bus_rd: 128711, bus_wb: 6499, bus_upd: 17688
```

./coherence my bodytrack 8192 2 32       

```                                                                       
> set protocol: my
> opening bodytrack_four/*
> successfully opened 4 input files
> cache_size: 8192 Bytes
> associativity: 2
> block_size: 32 Bytes
> simulation init...
> core0 init...
> core1 init...
> core2 init...
> core3 init...

> core2 finish_my at cycle 18791522
> core2 compute_cycle: 17556877, bus_idle: 19433, mem_idle: 1101182
> core2 load_inst_num: 74523, store_inst_num: 43175
> core2 private_load: 50375, shared_load: 24148
> core2 private_store: 42723, shared_store: 452
> core2 cache_hit: 109240, cache_miss: 8458, hit_rate: 0.928138

> core1 finish_my at cycle 42444243
> core1 compute_cycle: 17120545, bus_idle: 278961, mem_idle: 21758940
> core1 load_inst_num: 2388005, store_inst_num: 899247
> core1 private_load: 1606690, shared_load: 781315
> core1 private_store: 898346, shared_store: 901
> core1 cache_hit: 3096619, cache_miss: 190633, hit_rate: 0.942008

> core0 finish_my at cycle 42509007
> core0 compute_cycle: 17729254, bus_idle: 282538, mem_idle: 21230388
> core0 load_inst_num: 2380720, store_inst_num: 889412
> core0 private_load: 1562253, shared_load: 818467
> core0 private_store: 887303, shared_store: 2109
> core0 cache_hit: 3083485, cache_miss: 186647, hit_rate: 0.942924

> core3 finish_my at cycle 43116508
> core3 compute_cycle: 17140113, bus_idle: 296016, mem_idle: 22360062
> core3 load_inst_num: 2416052, store_inst_num: 908867
> core3 private_load: 1598388, shared_load: 817664
> core3 private_store: 906591, shared_store: 2276
> core3 cache_hit: 3129465, cache_miss: 195454, hit_rate: 0.941215

> simulation finished at cycle 43116508.
> bus_rd: 580542, bus_wb: 82548, bus_upd: 24844
```

./coherence my fluidanimate 8192 2 32

```
> set protocol: my
> opening fluidanimate_four/*      
> successfully opened 4 input files
> cache_size: 8192 Bytes
> associativity: 2
> block_size: 32 Bytes
> simulation init...
> core0 init...
> core1 init...
> core2 init...
> core3 init...

> core3 finish_my at cycle 39789821
> core3 compute_cycle: 11301515, bus_idle: 351208, mem_idle: 25728432
> core3 load_inst_num: 1832174, store_inst_num: 579291
> core3 private_load: 1374608, shared_load: 457566
> core3 private_store: 564717, shared_store: 14574
> core3 cache_hit: 2248296, cache_miss: 163169, hit_rate: 0.932336

> core1 finish_my at cycle 40270832
> core1 compute_cycle: 11290799, bus_idle: 384470, mem_idle: 26190160
> core1 load_inst_num: 1821846, store_inst_num: 585998
> core1 private_load: 1365916, shared_load: 455930
> core1 private_store: 573839, shared_store: 12159
> core1 cache_hit: 2243014, cache_miss: 164830, hit_rate: 0.931545

> core0 finish_my at cycle 42814508
> core0 compute_cycle: 11337782, bus_idle: 337922, mem_idle: 28563040
> core0 load_inst_num: 1832392, store_inst_num: 744111
> core0 private_load: 1385959, shared_load: 446433
> core0 private_store: 736142, shared_store: 7969
> core0 cache_hit: 2397203, cache_miss: 179300, hit_rate: 0.930410

> core2 finish_my at cycle 45019481
> core2 compute_cycle: 11337671, bus_idle: 328514, mem_idle: 30752078
> core2 load_inst_num: 1838008, store_inst_num: 766181
> core2 private_load: 1390596, shared_load: 447412
> core2 private_store: 764365, shared_store: 1816
> core2 cache_hit: 2411779, cache_miss: 192410, hit_rate: 0.926115

> simulation finished at cycle 45019481.
> bus_rd: 698237, bus_wb: 406395, bus_upd: 50648
```
