# Full Investigation of Emerging Caching Policies

## Project Overview
This project implements and evaluates four emerging caching policies—**LFU (Least Frequently Used)**, **LIRS (Low Inter-reference Recency Set)**, **ARC (Adaptive Replacement Cache)**, and **CACHEUS**—within a C++ simulation framework. The goal is to compare their performance metrics, such as Hit Ratio and Dirty Page Eviction, against standard industry workloads (MSR and SNIA traces).

Developed for **EEDG/CE 6304 - Computer Architecture** at **UT Dallas**.

### Implemented Policies
1.  **LFU (Least Frequently Used):**
    * Evicts blocks with the lowest access count.
    * **Pro:** Ideal for skewed workloads.
    * **Con:** Vulnerable to "cache pollution" from one-time scans.

2.  **LIRS (Low Inter-reference Recency Set):**
    * Prioritizes Inter-Reference Recency (IRR) over simple frequency.
    * Distinguishes between "hot" LIR blocks and "cold" HIR blocks to prevent pollution.
    * **Key Result:** Achieved 0 dirty page evictions in the `mds_1` trace test.

3.  **ARC (Adaptive Replacement Cache):**
    * Dynamically balances Recency (L1) and Frequency (L2) using ghost lists (T1, T2).
    * Self-tuning parameter `p` adapts to the workload's characteristics on the fly.
    * **Key Result:** Achieved ~99% hit ratio on the `prxy_0` trace.

4.  **CACHEUS:**
    * A write-aware policy designed for storage systems.
    * Splits cache into Read and Write segments, optimizing for both hit ratio and write traffic reduction.

## Directory Structure
* `src/`: Contains policy headers (`.h`) and implementations (`.cpp`) for LFU, LIRS, ARC, and CACHEUS.
* `traces/`: (Not included) Supports MSR and SNIA trace formats (`.csv`).

## Compilation & Usage
To compile the simulation, ensure you have a C++11 compliant compiler.
```bash
g++ -std=c++11 -o cache main.cpp lru.cpp lfu.cpp lirs.cpp arc.cpp cacheus.cpp
