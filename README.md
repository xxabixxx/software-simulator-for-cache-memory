# software-Simulator for cache memory
Computer Architecture

## Developing a Simulation Software for Cache Memory
The simulated cache will have the general functionality, but with a new Replacement Policy that works as follows. \
Each Cache Set is divided into two groups:
1. One group contains the HIGH PRIORITY lines of the set
2. The other group contains the LOW PRIORITY lines of the set

### How is priority established?
- If a line is accessed again after the initial access that fetches it into the cache, it is promoted to the HIGH PRIORITY group.
- If a line is not accessed for sufficiently long (T cache accesses) after being moved to the HIGH PRIORITY group, it is moved to the LOW PRIORITY group.
- Within a priority group, the Least Recently Used policy may be used to manage the lines.
- Assumption: A cache line is moved to Higher Priority level unless it is accessed (Even if the Higher Priority level has empty blocks and the Lower Priority level is full).

### Inputs
1. Cache Size
2. Cache line/block size
3. Cache associativity
4. The value of T
5. A sequence of memory access requests: Memory address, R or W (for Read or Write), and Data (if Write)

### Outputs
1. The complete content of the cache (Data, Tag, and Control bits of every cache line) in each Way after the memory access sequence is complete.
2. Cache statistics:
    1. Number of Accesses
    2. Number of Reads, Read Hits, and Read Misses
    3. Number of Writes, Write Hits, and Write Misses
    4. Hit Ratio
