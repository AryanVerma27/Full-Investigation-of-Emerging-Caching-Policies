/* lirs.h - LIRS (Low Inter-reference Recency Set) Cache Policy */
#include <string>
#include <unordered_map>
#include <list>
#include <set> // To maintain the set of LIR pages (or LIR block keys)
using namespace std; 
#ifndef _lirs_H
#define _lirs_H

// Forward declaration of the R-Stack Entry
struct RStackEntry;

class LIRSCache
{
private:
    int csize; // Maximum capacity of cache
    int lir_size; // Target size for LIR set (often based on a percentage of csize)
    
    // R-Stack: Tracks recency. Implemented as a list of block keys.
    std::list<long long int> R; 
    
    // Key-to-Entry Map: Maps a block key to its corresponding iterator in the R-Stack (R).
    // The long long int is the block key, the iterator is its position in R.
    std::unordered_map<long long int, std::list<long long int>::iterator> R_map;

    // Cache Residency Check: Maps block key to a boolean indicating if it is CURRENTLY in the cache (LIR or resident HIR)
    std::unordered_map<long long int, bool> resident_map;

    // LIR Set: Uses a set for O(logN) lookup to check if a block is LIR.
    std::set<long long int> LIR_set;

    // HIR Set (Non-resident): Tracks HIR keys that are NOT currently in the cache 
    // but whose history we want to keep (similar to a ghost list).
    std::list<long long int> HIR_nonresident_list;

    // Map to quickly check the status of a key (LIR, resident HIR, non-resident HIR)
    enum Status {NON_RESIDENT_HIR, RESIDENT_HIR, LIR};
    std::unordered_map<long long int, Status> key_status_map;

    // Statistics (Similar to LRU/LFU)
    std::unordered_map<long long int, string> accessType; 
    long long int calls, total_calls;
    long long int hits, total_hits;
    long long int readHits; 
    long long int writeHits; 
    long long int evictedDirtyPage; 
    long long int migration, total_migration;

    // Helper functions for the LIRS policy
    void prune_stack();
    void adjust_LIR_size();
    void evict_HIR_block();
    void evict_LIR_block();
    
public:
    LIRSCache(int);
    ~LIRSCache();
    void refer(long long int, string);
    void display();
    void cachehits();
    void refresh();
    void summary();
};
#endif