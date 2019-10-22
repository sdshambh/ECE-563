# ECE-563
Microarchitecture

Project 1: Cache Design, Memory Hierarchy Design
--------------------------------------------------

Implement a flexible cache and memory hierarchy simulator and use it to compare the performance, area, and energy of different 
memory hierarchy configurations, using a subset of the SPEC-2000 benchmark suite
CACHE should use the LRU (least-recently-used) replacement policy.
CACHE should use the WBWA (write-back + write-allocate) write policy.
CACHE receives a read or write request from whatever is above it in the memory hierarchy (either the CPU or another cache). 

The only situation where CACHE must interact with the next level below it (either another CACHE or main memory) is when the read or write request misses in CACHE. 
When the read or write request misses in CACHE, CACHE must “allocate” the requested block so that the read or write can be performed.

After servicing a read or write request, whether the corresponding block was in the cache already (hit) or had just been allocated (miss), remember to update other state. 
This state includes LRU counters affiliated with the set as well as the valid and dirty bits affiliated with the requested block.

Enable or disable victim cache should be an added feature in the cache.

Project 2: Branch Prediction
-----------------------------
Construct a branch predictor simulator and use it to evaluate different configurations of branch predictors.
Model a gshare branch predictor with parameters {m, n}, where:
1) m is the number of low-order PC bits used to form the prediction table index.
Note: discard the lowest two bits of the PC, since these are always zero, i.e., use bits m+1 through 2 of the PC.
2) n is the number of bits in the global branch history register. 
Note: n ≤ m. Note: n may equal zero, in which case we have the simple bimodal branch predictor.

1)When n=0, the gshare predictor reduces to a simple bimodal predictor.
2)When n > 0, there is an n-bit global branch history register. 
In this case, the index is based on both the branch’s PC and the global branch history register
3)Model a hybrid predictor that selects between the bimodal and the gshare predictors, using a chooser table of 2k 2-bit counters. 
All counters in the chooser table are initialized to 1 at the beginning of the simulation.

Project 3: Dynamic Instruction Scheduling
------------------------------------------

Construct a simulator for an out-of-order superscalar processor that fetches and issues N instructions per cycle. 
Only the dynamic scheduling mechanism will be modeled in detail, i.e., perfect caches and perfect branch prediction are assumed.
Architecture modeled is in spec file.
