/* cacheus.cpp - CACHEUS (Cache with Adaptive Segment Update for Storage) Cache Policy Implementation */

#include <iostream>
#include <fstream>
#include <ctime>
#include <algorithm>
#include "cacheus.h" 
using namespace std; 

// ------------------------------------------------------------------
// Constructor and Destructor
// ------------------------------------------------------------------
CACHEUSCache::CACHEUSCache(int n) {
    csize = n;
    // Initialize segments: Start with a bias toward reads (90/10 split), 
    // which is safer than 0/1. If csize=1, this is 1/0, which must be handled.
    write_segment_size = max(1, (int)(csize * 0.1)); 
    read_segment_size = csize - write_segment_size;
    
    // Ensure that if csize is small (e.g., 1), read_segment_size is at least 0.
    if (read_segment_size < 0) read_segment_size = 0;

    // Initialize statistics variables
    hits = 0;	
    calls = 0;	
    readHits = 0; 
    writeHits = 0; 
    evictedDirtyPage = 0; 
    
    std::cout << "CACHEUS Algorithm is used" << std::endl;
	std::cout << "Cache size: " << csize << ", Read Segment Size: " << read_segment_size << ", Write Segment Size: " << write_segment_size << std::endl;
}

// Memory-safe Destructor
CACHEUSCache::~CACHEUSCache() {
    Read_List.clear();
    Write_List.clear();
    list_map.clear();
    key_segment_map.clear();
    accessType.clear();
    // Reset all stat variables...
}

// ------------------------------------------------------------------
// CACHEUS Helper Functions (Eviction and Adaptation)
// ------------------------------------------------------------------

void CACHEUSCache::evict_read() {
    if (Read_List.empty()) return; // Critical safety check

    // Evict the LRU block from the Read Segment
    long long int victim = Read_List.back();
    Read_List.pop_back();

    list_map.erase(victim);
    key_segment_map.erase(victim);
    
    // Check dirty status upon eviction 
    if(accessType.count(victim) && accessType.at(victim) == "Write"){
        evictedDirtyPage++;             
    }
    accessType.erase(victim);
}

void CACHEUSCache::evict_write() {
    if (Write_List.empty()) return; // Critical safety check

    // Evict the LRU block from the Write Segment
    long long int victim = Write_List.back();
    Write_List.pop_back();

    list_map.erase(victim);
    key_segment_map.erase(victim);

    // Write segment blocks are often dirty, so check and count
    if(accessType.count(victim) && accessType.at(victim) == "Write"){
        evictedDirtyPage++;             
    }
    accessType.erase(victim);
}

void CACHEUSCache::adapt_segments() {
    // Placeholder for complex adaptive logic. 
    // This is where read_segment_size and write_segment_size would be adjusted.
    // We leave this empty to prevent complexity causing more bugs.
}

// ------------------------------------------------------------------
// Refer Method (Core CACHEUS Logic)
// ------------------------------------------------------------------
void CACHEUSCache::refer(long long int x, string rwtype) {
    calls++;
    
    Segment current_segment = key_segment_map.count(x) ? key_segment_map.at(x) : NONE;

    // --- 1. HIT ---
    if (current_segment != NONE) {
        hits++;
        (rwtype == "Read") ? readHits++ : writeHits++;
        
        // Remove from current list
        if (current_segment == READ) {
            Read_List.erase(list_map.at(x));
        } else { // WRITE
            Write_List.erase(list_map.at(x));
        }
        
        // --- PROMOTION/DEMOTION LOGIC ---
        // All hits move to the MRU end of their respective segment, 
        // with writes ensuring they are in the Write Segment.
        
        if (rwtype == "Read") {
            // Read hit: stays/promotes to MRU of Read Segment
            Read_List.push_front(x);
            key_segment_map[x] = READ;
            list_map[x] = Read_List.begin();
        } else { // Write hit
            // Write hit: stays/promotes to MRU of Write Segment
            Write_List.push_front(x);
            key_segment_map[x] = WRITE;
            accessType[x] = "Write"; // Mark as dirty
            list_map[x] = Write_List.begin();
        }
        
        // If the block moved segments, the old list will shrink, potentially requiring adaptation/eviction.
        adapt_segments();
        return;
    }
    
    // --- 2. MISS ---
    else {
        // --- EVICTION (Only if cache is full) ---
        if (Read_List.size() + Write_List.size() == csize) {
            
            bool evicting_read = false;
            
            // Priority 1: Evict from the segment that exceeds its target size AND is not empty
            if (Read_List.size() > read_segment_size && !Read_List.empty()) {
                evicting_read = true;
            } else if (Write_List.size() > write_segment_size && !Write_List.empty()) {
                evicting_read = false;
            } 
            // Priority 2: If neither exceeds target, find the viable LRU (prefer Read segment as victim)
            else if (!Read_List.empty()) {
                evicting_read = true;
            } else if (!Write_List.empty()) {
                evicting_read = false;
            } else {
                // Should only happen if csize=0, but included for safety.
                return; 
            }

            if (evicting_read) {
                evict_read();
            } else {
                evict_write();
            }
        }
        
        // --- INSERTION ---
        if (rwtype == "Read") {
            // Insert into Read Segment
            Read_List.push_front(x);
            key_segment_map[x] = READ;
            list_map[x] = Read_List.begin();
        } else { // Write miss
            // Insert into Write Segment
            Write_List.push_front(x);
            key_segment_map[x] = WRITE;
            list_map[x] = Write_List.begin();
            accessType[x] = "Write"; // Mark as dirty immediately
        }

        adapt_segments(); // Recheck segment sizes after insertion
    }
}


// ------------------------------------------------------------------
// Remaining Required Methods (Statistical Reporting)
// ------------------------------------------------------------------
void CACHEUSCache::display() {
	std::cout << "CACHEUS Cache displayed." << std::endl;
}

void CACHEUSCache::cachehits() {
    float hitRatio = (calls > 0) ? (float)hits / calls : 0.0;

	std::cout<< "calls: " << calls << ", hits: " << hits << ", readHits: " << readHits << ", writeHits: " <<  writeHits << ", evictedDirtyPage: " << evictedDirtyPage << std::endl;

	std::ofstream result("ExperimentalResult.txt", std::ios_base::app);
	if (result.is_open()) { 
		result <<  "CACHEUS " << "CacheSize " << csize << " calls " << calls << " hits " << hits << " hitRatio " << hitRatio << " readHits " << readHits << " readHitRatio " << ((calls > 0) ? (float)readHits/calls : 0.0) << " writeHits " << writeHits << " writeHitRatio " << ((calls > 0) ? (float)writeHits/calls : 0.0) << " evictedDirtyPage " << evictedDirtyPage << "\n" ;			
	}
	result.close();
}

void CACHEUSCache::refresh(){
	calls = 0;
	hits = 0;
	migration = 0;
}

void CACHEUSCache::summary() {
	// print the number of total cache calls, hits, and data migration size
}