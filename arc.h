/* arc.h - ARC (Adaptive Replacement Cache) Cache Policy */
#include <string>
#include <unordered_map>
#include <list>
#include <algorithm>
using namespace std; 
#ifndef _arc_H
#define _arc_H

class ARCCache
{
private:
    int csize; // Maximum capacity of the cache (C)
    int p;     // The 'pivot' or target size for the L1/T1 lists (0 <= p <= C)

    // L1: List of recently referenced pages not seen before (L1 is an LRU list)
    std::list<long long int> L1; 
    // T1: Ghost list corresponding to L1 (history of L1 pages)
    std::list<long long int> T1;

    // L2: List of frequently referenced pages (L2 is an LRU list)
    std::list<long long int> L2;
    // T2: Ghost list corresponding to L2 (history of L2 pages)
    std::list<long long int> T2;
    
    // Maps key to iterator in its respective list (L1, T1, L2, or T2)
    std::unordered_map<long long int, std::list<long long int>::iterator> list_map;

    // Maps key to its current list/set (e.g., '1' for L1, '2' for L2, etc.)
    enum ListSet {NONE, L1_SET, T1_SET, L2_SET, T2_SET};
    std::unordered_map<long long int, ListSet> key_set_map;

    // Statistics (Similar to LRU/LFU)
    std::unordered_map<long long int, string> accessType; 
    long long int calls, total_calls;
    long long int hits, total_hits;
    long long int readHits; 
    long long int writeHits; 
    long long int evictedDirtyPage; 
    long long int migration, total_migration;

    // Helper functions for the ARC policy
    void replace(); // <-- CORRECTED DECLARATION
    void clean_ghost_list(std::list<long long int>& T_list_to_check);
    
public:
    ARCCache(int);
    ~ARCCache();
    void refer(long long int, string);
    void display();
    void cachehits();
    void refresh();
    void summary();
};
#endif