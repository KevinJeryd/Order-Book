# Order Book
High-performance C++ limit order book with a matching engine, benchmarked at 3.5M orders/sec.

# Overview
An order book implementation for matching buyers and sellers, supporting both full and partial filling. Exposing a public API for adding and cancelling orders, as well as querying information such as current best bid, best ask and spread for analysis.

# Architecture
The order book's internal bids and asks container currently uses sorted maps with the price as key and a queue of the orders at that pricepoint as the underlying data structure for matching best bid and best ask in O(1) time and respecting time of order when several orders are put at the same price.

To allow for quick deletion an additional unsorted map has been used for instant fetching of information that allows to locate the correct order. While fetching the correct map and key is instant, the implementation currently has to iterate to get the correct index in the queue for that price point, at most n times, where n is the length of the queue, if the order is at the furthest back of the queue. Since the queue is ordered in time, and not by order index, searching algorithms can't be used to decrease this time, for now.

Using the ordered maps also allows for the instant querying of best bid, ask and spread.

# Performance
## Current Implementation (Direct-Indexed Array)
| Metric | Value |
|--------|-------|
| Min Time | 1762.5ms |
| Avg Time | 2095.96ms |
| Max Time | 4650.07ms |
| Peak Throughput | 0.57M orders/sec |
| Avg Throughput | 0.48M orders/sec |
| Min Throughput | 0.22M orders/sec |

## Previous Implementation (std::map)
The order book currently handles around 3.5 million orders per second with the map implementation. See benchmark numbers below.
| Metric | Value |
|--------|-------|
| Min Time | 283.35ms |
| Avg Time | 287.64ms |
| Max Time | 295.92ms |
| Peak Throughput | 3.53M orders/sec |
| Avg Throughput | 3.48M orders/sec |
| Min Throughput | 3.38M orders/sec |

# Running the application
To run the application, first ensure that you have CMake installed.
Following commands are done from project root.

First of all, to generate the build files run `cmake -S . -B build`

Building can be done by running `cmake --build build`

Run the test through `./build/orderbook_tests`

Run the benchmark through `./build/benchmark`


# Optimization Experiments
## Direct-indexed static array — Hypothesis
The profiling data showed that the map's 0.2% cache miss rate was not a significant bottleneck. The real cost was the O(log n) tree traversal on every insertion and lookup. The direct-indexed static array implementation eliminates this entirely by using direct index calculation. Given a price, the index is computed as `price - base_price` in O(1) with no searching, no tree traversal, and no element shifting.

Since prices cluster in a narrow range (roughly 120 ticks), a fixed array of 500 slots covers the realistic price range with room to spare. The array is contiguous in memory, so cache locality is maintained. Combined with O(1) insertion and lookup, the direct-indexed array should outperform both the map and the vector implementations.

Only benchmarking will confirm this.

## Vector Implementation — Hypothesis and Results (See branch for impl)
### Hypothesis
Currently the map implementation suffers from constant pointer chasing and cache misses, which leads to always having to fetch the next bid or ask in ram, even though we know it's the next one in line, since the map is ordered. To bypass these misses a sorted vector can be used instead. The vector implementation would fetch the entire cache line and put it into the L1 cache, which would make the upcoming asks and bids instantaneous. Furthermore, the CPUs prefetcher would detect a sequential access pattern and start loading the next cache line as well, removing the trips to ram entirely.

However, worth mentioning, is that insertion will go from O(log n), which the maps underlying tree structure allows for, to O(n), since the array might have to move all the values to the right if inserting from a new best value. The argument for why the array still results in better performance despite its worse time complexity is that the number of price levels in a realistic scenario would be small enough that the cache miss reduction will make up for moving N elements.

However, only benchmarking will show if this is true.

### Results
| Metric | Value |
|--------|-------|
| Min Time | 1762.5ms |
| Avg Time | 2095.96ms |
| Max Time | 4650.07ms |
| Peak Throughput | 0.57M orders/sec |
| Avg Throughput | 0.48M orders/sec |
| Min Throughput | 0.22M orders/sec |

The vector implementation underperformed despite better theoretical cache locality because the O(n) element shifting on insertion dominated the O(log n) tree traversal cost of the map. With prices clustered in a narrow range, the book maintains roughly 120 active price levels, meaning every insertion shifts up to 120 elements. The cache benefit exists but is overwhelmed by this cost.

### Cache Analysis Comparison

Profiled using `valgrind --tool=cachegrind` on 1,000,000 orders.

| Metric | std::map | sorted std::vector |
|--------|----------|--------------------|
| Total data references | 22.9 billion | 230 billion |
| D1 cache miss rate | 0.2% | 0.016% |
| LLd cache miss rate | 0.0% | 0.0% |

The vector implementation confirmed lower cache miss rates as predicted, validating the cache locality theory. However, the O(n) element shifting generated 10x more total memory operations, completely overwhelming the cache benefit. The map's 0.2% D1 miss rate, while higher, is still low in absolute terms — only 38 million misses across 22.9 billion references.

**Conclusion:** for this workload, reducing total memory operations matters more than reducing cache miss rate. The price ladder implementation targets both, O(1) insertion eliminates shifting, and contiguous memory maintains cache locality.
