# 1092 計算機組織 Project2

## Q1. GEM5 + NVMAIN BUILD-UP (40%)
照著教學走即可完成
## Q2. Enable L3 last level cache in GEM5 + NVMAIN (15%)

### 說明
- `configs/common/Caches.py`, `configs/common/CacheConfig`：增加l3Cache
- `src/mem/xbar.py`, `src/cpu/BaseCPU.py`：把l3裝上CPU 
- `configs/common/Options.py`增加`--l3cache`: 讓指令可以設定l3

### 測試指令
```sh
 ./build/X86/gem5.opt configs/example/se.py -c tests/test-progs/hello/bin/x86/linux/hello \
--cpu-type=TimingSimpleCPU \
--caches --l2cache --l3cache \
--l1i_size=32kB --l1d_size=32kB --l2_size=128kB --l3_size=1MB \
--l3_assoc=2 \
--mem-type=NVMainMemory \
--nvmain-config=../NVmain/Config/PCM_ISSCC_2012_4GB.config
```

## Q3. Config last level cache to 2-way and full-way associative cache and test performance (15%)
### 說明
在指令的地方利用`--l3_assoc`改寫l3 cache的associativity
* 2-way改為 2
* full-way改為64(因由Options.py預設block size為64)
![](https://i.imgur.com/cYp4KeR.png)

並使用`--l1i_size`, `--l1d_size`, `--l2_size`, `--l3_size`改為benchmark要求的cache size規定

### 測試指令-使用quicksort
- 2-way 
```shell=
 ./build/X86/gem5.opt configs/example/se.py -c benchmark/quicksort \
--cpu-type=TimingSimpleCPU \
--caches --l2cache --l3cache \
--l1i_size=32kB --l1d_size=32kB --l2_size=128kB --l3_size=1MB \
--l3_assoc=2 \
--mem-type=NVMainMemory \
--nvmain-config=../NVmain/Config/PCM_ISSCC_2012_4GB.config

```
- full-way
```shell=
 ./build/X86/gem5.opt configs/example/se.py -c benchmark/quicksort \
--cpu-type=TimingSimpleCPU \
--caches --l2cache --l3cache \
--l1i_size=32kB --l1d_size=32kB --l2_size=128kB --l3_size=1MB \
--l3_assoc=64 \
--mem-type=NVMainMemory \
--nvmain-config=../NVmain/Config/PCM_ISSCC_2012_4GB.config
```

執行的結果放置於result/associativity資料夾中

## Q4. Modify last level cache policy based on RRIP (15%)
### 說明
在`configs/common/Caches.py`中將L3Cache 的 replacement_policy改為`RRIPRP()`

可以去`src/mem/cache/replacement_policies/ReplacementPolicies.py`中查看有哪些policy可以使用
* [gem5 replacment policy 支援列表](https://www.gem5.org/documentation/general_docs/memory_system/replacement_policies/)
```c=
class L3Cache(Cache):
    assoc = 64
    tag_latency = 32
    data_latency = 32
    response_latency = 32
    mshrs = 32
    tgts_per_mshr = 24
    write_buffers = 16
    replacement_policy = RRIPRP()
```

### 測試指令
* 編譯助教給的demo code
```
gcc --static demo.c -o demo
```
* 執行
```
./build/X86/gem5.opt configs/example/se.py -c benchmark/demo \
--cpu-type=TimingSimpleCPU \
--caches --l2cache --l3cache \
--l1i_size=32kB --l1d_size=32kB --l2_size=128kB --l3_size=1MB \
--l3_assoc=2 \
--mem-type=NVMainMemory \
--nvmain-config=../NVmain/Config/PCM_ISSCC_2012_4GB.config
```


## Q5. Test the performance of write back and write through policy based on 4-way associative cache with isscc_pcm(15%)
### Write Back說明: [參考官方文件](https://www.gem5.org/documentation/general_docs/memory_system/gem5_memory_system/)
gem5預設即為write back，故不須做修改即可執行write back，以下說明gem5的架構

**1. CPU Access**
![](https://i.imgur.com/0Fqe6VE.png)
* 將read request和write request分開存在不同buffer(queue):
    * **MSHR(Miss Status and Handling Register)**: 儲存尚未完成的memory read request
        * Cached Read misses (invalid flag)
        * Cached Write misses (invalid flag)
        * Uncached reads
    * **WriteBuffer:** 課堂上講到的write buffer，儲存尚未完成的memory write request
        * Uncached writes
        * 被evicted(& dirty)掉cache lines的Writeback
* 當write buffer或MSHR滿的時候，CPU將無法存取cache直到出現空位為止

**2. 寫入Memory**

![](https://i.imgur.com/QNPMAd0.png)
* 每個CPU access都會給予一個順序編號，之後在MSHR和WriteBuffer會依照順序編好去執行。
* 然而若是access相同的block，所有那個block的request都會一起執行
    * 如圖中READ#1，#1、#5、#10會一起執行，因此#5會比#3快完成
### Write Through 說明
有做過以下嘗試，但似乎沒有成功
* 在CPU access時將所有的write packet加上write through的flag
```c=
BaseCache::access(PacketPtr pkt, CacheBlk *&blk, Cycles &lat,
                  PacketList &writebacks)
{
    // sanity check
    assert(pkt->isRequest());

    chatty_assert(!(isReadOnly && pkt->isWrite()),
                  "Should never see a write in a read-only cache %s\n",
                  name());

    //------------------------START CHANGE TO WRITE THROUGH---------------------
    if(pkt->isWrite() && (pkt->isEviction() || pkt->cmd == MemCmd::WriteClean)){
        pkt->setWriteThrough();
    }
    //------------------------END CHANGE TO WRITE THROUGH-----------------------

...

}
```
* 在NVmain的`PCM_ISSCC_2012_4GB`設定檔，將memory更改為write through
```

;----------------------------START CHANGE TO WRITE THROUGH-----------------------

tRP 0    ; Precharge isn't needed. Writes occur only if needed
         ; and take tWP time during a precharge (write-back)
         ; or immediately (write-through)

;----------------------------END CHANGE TO WRITE THROUGH-------------------------

```

* 將write buffer設為0，不存到buffer直接往下送，結果無法執行完benchmark
```
class L2Cache(Cache):
    assoc = 8
    tag_latency = 20
    data_latency = 20
    response_latency = 20
    mshrs = 20
    tgts_per_mshr = 12
    write_buffers = 0

class L3Cache(Cache):
    assoc = 64
    tag_latency = 32
    data_latency = 32
    response_latency = 32
    mshrs = 32
    tgts_per_mshr = 24
    write_buffers = 0
```
### 測試指令-使用multiply
```shell=
./build/X86/gem5.opt configs/example/se.py -c benchmark/multiply \
--cpu-type=TimingSimpleCPU \
--caches --l2cache --l3cache \
--l1i_size=32kB --l1d_size=32kB --l2_size=128kB --l3_size=1MB \
--l3_assoc=4 \
--mem-type=NVMainMemory \
--nvmain-config=../NVmain/Config/PCM_ISSCC_2012_4GB.config
```

## Bonus. Design last level cache policy to reduce the energy consumption
* 有嘗試使用pseudo LRU但似乎效能比baseline LRU差，pesudo LRU是使用[https://github.com/moulaskar/gem5](https://github.com/moulaskar/gem5)提供的方法，檔案放在`gem5/src/mem/cache/replacement_policies/`的`lru_ipv.hh`和`lru_ipv.cc`
* 參考論文
[Insertion and Promotion for Tree-Based PseudoLRU Last-Level Caches](http://taco.cse.tamu.edu/pdfs/p284-jimenez.pdf)