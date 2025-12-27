/* arc.cpp - ARC (Adaptive Replacement Cache) Cache Policy Implementation */

#include <iostream>
#include <fstream>
#include <ctime>
#include <algorithm>
#include "arc.h" 
using namespace std; 

// ------------------------------------------------------------------
// Constructor and Destructor
// ------------------------------------------------------------------
ARCCache::ARCCache(int n) {
    csize = n;
    p = 0; 
    
    // Initialize statistics variables
    hits = 0;	
    calls = 0;	
    readHits = 0; 
    writeHits = 0; 
    evictedDirtyPage = 0; 
    
    std::cout << "ARC Algorithm is used" << std::endl;
	std::cout << "Cache size: " << csize << ", Initial pivot p: " << p << std::endl;
}

// Memory-safe Destructor
ARCCache::~ARCCache() {
    L1.clear();
    T1.clear();
    L2.clear();
    T2.clear();
    list_map.clear();
    key_set_map.clear();
    accessType.clear();
}

// ------------------------------------------------------------------
// ARC Helper Function: Clean Ghost List (Helper for replace)
// ------------------------------------------------------------------

void ARCCache::clean_ghost_list(std::list<long long int>& T_list_to_check) {
    // Ensures T1.size() + T2.size() <= C
    while (T1.size() + T2.size() > csize) {
        
        long long int victim;
        
        // Prioritize removal from T2 (less valuable history)
        if (!T2.empty()) {
            victim = T2.back();
            T2.pop_back(); 
        } 
        // If T2 is empty, remove from T1
        else if (!T1.empty()) {
            victim = T1.back();
            T1.pop_back(); 
        } else {
            break; // Should not happen
        }
        
        // Critical: Safely remove victim from maps
        if (list_map.count(victim)) {
            list_map.erase(victim);
        }
        if (key_set_map.count(victim)) {
            key_set_map.erase(victim);
        }
    }
}

// ------------------------------------------------------------------
// ARC Helper Function: Replacement logic 
// ------------------------------------------------------------------

void ARCCache::replace() {
    long long int victim;
    
    // Evict from L1 if L1 is larger than p 
    if (L1.size() > p) { 
        // L1 must not be empty here since L1.size() > p >= 0
        victim = L1.back();
        
        // 1. Remove from L1 
        L1.pop_back();
        
        // 2. Remove from maps (MUST occur before re-adding to T1)
        list_map.erase(victim);
        key_set_map.erase(victim);
        
        // 3. Add to T1 
        T1.push_front(victim);
        list_map[victim] = T1.begin();
        key_set_map[victim] = T1_SET;
        
        // Check dirty eviction status
        if(accessType.count(victim) && accessType.at(victim) == "Write"){
            evictedDirtyPage++;             
        }
        // Access type moves to the ghost list, but we erase it if it was dirty
        accessType.erase(victim); 
    } 
    // Evict from L2
    else if (!L2.empty()) { // Added check to ensure L2 is not empty
        victim = L2.back();

        // 1. Remove from L2
        L2.pop_back();
        
        // 2. Remove from maps
        list_map.erase(victim);
        key_set_map.erase(victim);

        // 3. Add to T2 
        T2.push_front(victim);
        list_map[victim] = T2.begin();
        key_set_map[victim] = T2_SET;
        
        // Check dirty eviction status
        if(accessType.count(victim) && accessType.at(victim) == "Write"){
            evictedDirtyPage++;             
        }
        accessType.erase(victim); 
    } else {
        // Should not happen if L1+L2 = C, but as a safety break:
        return;
    }

    // After replacement, ensure ghost lists are under capacity C
    clean_ghost_list(T1); 
}

// ------------------------------------------------------------------
// Refer Method (Core ARC Logic)
// ------------------------------------------------------------------
void ARCCache::refer(long long int x, string rwtype) {
    calls++;
    
    ListSet current_set = key_set_map.count(x) ? key_set_map.at(x) : NONE;

    // === 1. HIT in L1 or L2 (Resident Cache Hit) ===
    if (current_set == L1_SET || current_set == L2_SET) {
        hits++;
        (rwtype == "Read") ? readHits++ : writeHits++;
        if (rwtype == "Write") accessType[x] = "Write";

        // 1. Remove from current list (L1 or L2)
        if (current_set == L1_SET) {
            L1.erase(list_map.at(x));
        } else { // L2_SET
            L2.erase(list_map.at(x));
        }
        
        // 2. Add to MRU end of L2
        L2.push_front(x);
        list_map[x] = L2.begin();
        key_set_map[x] = L2_SET;
        return;
    }
    
    // === 2. HIT in T1 or T2 (Ghost List Hit - Requires Insertion) ===
    else if (current_set == T1_SET || current_set == T2_SET) {
        
        // ADAPTATION STEP: Adjust the pivot 'p'
        if (current_set == T1_SET) {
            p = std::min(csize, p + 1); 
            T1.erase(list_map.at(x)); // Remove from T1 list
        } else { // T2_SET
            p = std::max(0, p - 1); 
            T2.erase(list_map.at(x)); // Remove from T2 list
        }
        
        // CRITICAL: Safely remove from maps (Must happen AFTER list erase and BEFORE list re-add)
        list_map.erase(x);
        key_set_map.erase(x);

        // Evict resident block if cache is full (L1+L2 = C)
        if (L1.size() + L2.size() == csize) {
            replace(); 
        }
        
        // Add x to L2 MRU end 
        L2.push_front(x);
        list_map[x] = L2.begin();
        key_set_map[x] = L2_SET;
        accessType[x] = rwtype; 
        clean_ghost_list(T1); 
        return;
    }
    
    // === 3. MISS (New Block - Requires Insertion) ===
    else {
        // Eviction logic if we need space for the new block.
        if (L1.size() + L2.size() == csize) {
            // Cache is full, need to evict a resident block
            replace(); 
        } else if (T1.size() + T2.size() == csize) {
            // Total ghost capacity exceeded, must evict a ghost (from T2)
            clean_ghost_list(T2); 
        }
        
        // Add new block x to L1 MRU end (L1 is the set for newly seen blocks)
        L1.push_front(x);
        list_map[x] = L1.begin();
        key_set_map[x] = L1_SET;
        accessType[x] = rwtype; 
    }
}


// ------------------------------------------------------------------
// Remaining Required Methods
// ------------------------------------------------------------------
void ARCCache::display() {
	std::cout << "ARC Cache displayed." << std::endl;
}

void ARCCache::cachehits() {
    float hitRatio = (calls > 0) ? (float)hits / calls : 0.0;

	std::cout<< "calls: " << calls << ", hits: " << hits << ", readHits: " << readHits << ", writeHits: " <<  writeHits << ", evictedDirtyPage: " << evictedDirtyPage << std::endl;

	std::ofstream result("ExperimentalResult.txt", std::ios_base::app);
	if (result.is_open()) { 
		result <<  "ARC " << "CacheSize " << csize << " calls " << calls << " hits " << hits << " hitRatio " << hitRatio << " readHits " << readHits << " readHitRatio " << ((calls > 0) ? (float)readHits/calls : 0.0) << " writeHits " << writeHits << " writeHitRatio " << ((calls > 0) ? (float)writeHits/calls : 0.0) << " evictedDirtyPage " << evictedDirtyPage << "\n" ;			
	}
	result.close();
}

void ARCCache::refresh(){
	calls = 0;
	hits = 0;
	migration = 0;
}

void ARCCache::summary() {
	// print the number of total cache calls, hits, and data migration size
}