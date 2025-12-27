/* lfu.h - LFU (Least Frequently Used) Cache Policy */
#include <string>
#include <unordered_map>
#include <list>
#include <map> // We will use std::map to organize by frequency
using namespace std; 
#ifndef _lfu_H
#define _lfu_H

class LFUCache
{
private:
    int csize; // maximum capacity of cache
    
    // 1. Tracks the frequency of each key (block address)
    unordered_map<long long int, int> keyFreq; 
    
    // 2. Tracks the key's position in the frequency list. 
    //    It maps key -> iterator to its position in freqList's std::list<long long int>
    //    This is analogous to LRU's 'ma' mapping key to its position in the list.
    unordered_map<long long int, list<long long int>::iterator> keyIterMap;
    
    // 3. Organizes keys by frequency. 
    //    The outer map: frequency (int) -> a list of keys (long long int) that have that frequency.
    //    The inner list is an LRU for keys with the same frequency.
    map<int, list<long long int>> freqList; 
    
    // Statistics (Copy from lru.h)
    unordered_map<long long int, string> accessType; 
    long long int calls, total_calls;
    long long int hits, total_hits;
    long long int readHits; 
    long long int writeHits; 
    long long int evictedDirtyPage; 
    long long int migration, total_migration;

public:
    LFUCache(int);
    ~LFUCache();
    void refer(long long int, string);
    void display();
    void cachehits();
    void refresh();
    void summary();
};
#endif