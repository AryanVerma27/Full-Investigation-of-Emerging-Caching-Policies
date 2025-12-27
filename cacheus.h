/* cacheus.h - CACHEUS (Cache with Adaptive Segment Update for Storage) Cache Policy */
#include <string>
#include <unordered_map>
#include <list>
#include <algorithm> 
using namespace std; 
#ifndef _cacheus_H
#define _cacheus_H

class CACHEUSCache
{
private:
    int csize; // Maximum total capacity of the cache (C)
    int write_segment_size; // Current size of the Write Segment
    int read_segment_size;  // Current size of the Read Segment

    // Read Segment: Managed by LRU
    std::list<long long int> Read_List; 
    
    // Write Segment: Managed by LRU
    std::list<long long int> Write_List; 
    
    // Maps key to iterator in its respective list (Read or Write)
    std::unordered_map<long long int, std::list<long long int>::iterator> list_map;

    // Maps key to its current segment
    enum Segment {NONE, READ, WRITE};
    std::unordered_map<long long int, Segment> key_segment_map;
    
    // Tracks dirty status (inherited from LRU/LFU)
    std::unordered_map<long long int, string> accessType; 

    // Statistics (Similar to others)
    long long int calls, total_calls;
    long long int hits, total_hits;
    long long int readHits; 
    long long int writeHits; 
    long long int evictedDirtyPage; 
    long long int migration, total_migration;

    // Helper functions for the CACHEUS policy
    void evict_read();
    void evict_write();
    void adapt_segments();
    
public:
    CACHEUSCache(int);
    ~CACHEUSCache();
    void refer(long long int, string);
    void display();
    void cachehits();
    void refresh();
    void summary();
};
#endif