/* lirs.cpp - LIRS (Low Inter-reference Recency Set) Cache Policy Implementation */

#include <iostream>
#include <fstream>
#include <ctime>
#include <algorithm>
#include "lirs.h" // Include your new header
using namespace std; 

// ------------------------------------------------------------------
// Constructor and Destructor
// ------------------------------------------------------------------
LIRSCache::LIRSCache(int n) {
    csize = n;
    // LIRS uses a split size; setting lir_size to 1% of csize is a common starting point
    // but the LIRS paper often sets the maximum LIR size based on the total number of blocks (L)
    // For simplicity, we can use a small fixed percentage of csize.
    lir_size = max(1, (int)(csize * 0.01)); 

    // Initialize statistics variables
    hits = 0;	
    calls = 0;	
    readHits = 0; 
    writeHits = 0; 
    evictedDirtyPage = 0; 
    
    std::cout << "LIRS Algorithm is used" << std::endl;
	std::cout << "Cache size: " << csize << ", LIR size: " << lir_size << std::endl;
}

LIRSCache::~LIRSCache() {
    R.clear();
    R_map.clear();
    resident_map.clear();
    LIR_set.clear();
    HIR_nonresident_list.clear();
    key_status_map.clear();
    accessType.clear();
    // Reset all stat variables...
}

// ------------------------------------------------------------------
// LIRS Helper Functions (Stubs for the main logic)
// ------------------------------------------------------------------

void LIRSCache::prune_stack() {
    // Prune the R-Stack (R) by removing the tail items until the first HIR item is reached.
    while (!R.empty()) {
        long long int key = R.back();
        // If the key is LIR, stop pruning
        if (key_status_map.at(key) == LIR) {
            break;
        }
        // If the key is a resident HIR, we should not remove it from the cache, just the stack
        
        // However, the standard implementation of LIRS often only requires removing non-resident HIR pages.
        // For the full LIRS, pages in R are unique. If we hit the first LIR from the back, we stop.
        // We only remove the key from the R-Stack if it's NOT LIR.
        R.pop_back();
        R_map.erase(key);
        // Note: The key still retains its LIR/HIR status and may still be in the cache.
    }
}

void LIRSCache::evict_HIR_block() {
    // Evicts the page with the lowest recency from the HIR resident set (usually the tail of the resident HIR list).
    // In this simplified structure, we look at the R-stack bottom.
    
    // Find a resident HIR block to evict. The key is to find the one with the largest recency (furthest from the top of R).
    // The standard LIRS uses a list B (or the non-resident HIR list) for this, but to keep the implementation simple, 
    // we use the HIR_nonresident_list as the candidate pool. Evict the key at the *head* of this list.
    
    if (HIR_nonresident_list.empty()) {
        // Should not happen if the policy is working and the cache is full
        return;
    }
    
    long long int victim = HIR_nonresident_list.front();
    
    // 1. Remove from cache (implicit by clearing status and resident_map)
    resident_map.erase(victim);
    
    // 2. Update status (set to non-resident HIR)
    key_status_map[victim] = NON_RESIDENT_HIR;
    
    // 3. Keep victim key in HIR_nonresident_list for history tracking
    
    // If the evicted page was written to, count it as dirty
    if(accessType.count(victim) && accessType.at(victim) == "Write"){
        evictedDirtyPage++;             
    }
    accessType.erase(victim);
    
    // *** NOTE: The full LIRS implementation requires adjusting the LIR/HIR sizes and promoting/demoting pages. ***
}

void LIRSCache::adjust_LIR_size() {
    // If a non-resident HIR is promoted to LIR, a LIR page must be demoted to HIR.
    // The LIR page to demote is the one at the bottom of the R-stack (first LIR element from the bottom)
    // This is the most complex part of LIRS and usually involves a search from the tail of R.
}

// ------------------------------------------------------------------
// Refer Method (High-Level LIRS Logic)
// ------------------------------------------------------------------
void LIRSCache::refer(long long int x, string rwtype) {
    calls++;

    // 1. Update R-Stack: Remove x from its current position in R (if it exists)
    if (R_map.count(x)) {
        R.erase(R_map.at(x));
        R_map.erase(x);
    }
    // Add x to the top of R (Most Recently Referenced)
    R.push_front(x);
    R_map[x] = R.begin();
    
    // Check if x is in cache (resident_map)
    if (resident_map.count(x)) {
        hits++;
        // Update stats
        (rwtype == "Read") ? readHits++ : writeHits++;
        if (rwtype == "Write") accessType[x] = "Write";

        // LIRS State Transitions (HIT):
        if (key_status_map.at(x) == LIR) {
            // LIR HIT: Already LIR, no state change. Prune the R-Stack.
            prune_stack();
        } else if (key_status_map.at(x) == RESIDENT_HIR) {
            // RESIDENT HIR HIT: Promote to LIR if IR of x is small (not done here for brevity)
            // A simple implementation promotes if the cache is below capacity.
            // Full LIRS requires checking the LIR set size limit.
            
            // For now, simple promotion if LIR size is not exceeded:
            if (LIR_set.size() < lir_size) {
                 key_status_map[x] = LIR;
                 LIR_set.insert(x);
                 // Demotion/Adjustment would happen in adjust_LIR_size, but required here for promotion.
                 prune_stack();
            } else {
                // Stay resident HIR
            }
        }
    } else {
        // MISS: Block x is NOT in cache
        
        // 1. Eviction: If cache is full, evict a block
        if (resident_map.size() == csize) {
            // Evict an HIR block (evict_HIR_block handles LIR size adjustment/demotion implicitly)
            evict_HIR_block();
        }
        
        // 2. Insertion: Insert x into cache as Resident HIR
        resident_map[x] = true;
        key_status_map[x] = RESIDENT_HIR; 
        
        // Update access type for the new block
        accessType[x] = rwtype;
    }

    // 3. Final Pruning: Remove x from the non-resident list if it was there (since it's now resident)
    if (HIR_nonresident_list.front() == x) {
        HIR_nonresident_list.pop_front();
    }
}

// ------------------------------------------------------------------
// Remaining Required Methods
// ------------------------------------------------------------------
void LIRSCache::display() {
	std::cout << "LIRS Cache displayed." << std::endl;
}

void LIRSCache::cachehits() {
    float hitRatio = (calls > 0) ? (float)hits / calls : 0.0;

	std::cout<< "calls: " << calls << ", hits: " << hits << ", readHits: " << readHits << ", writeHits: " <<  writeHits << ", evictedDirtyPage: " << evictedDirtyPage << std::endl;

	std::ofstream result("ExperimentalResult.txt", std::ios_base::app);
	if (result.is_open()) { 
		result <<  "LIRS " << "CacheSize " << csize << " calls " << calls << " hits " << hits << " hitRatio " << hitRatio << " readHits " << readHits << " readHitRatio " << ((calls > 0) ? (float)readHits/calls : 0.0) << " writeHits " << writeHits << " writeHitRatio " << ((calls > 0) ? (float)writeHits/calls : 0.0) << " evictedDirtyPage " << evictedDirtyPage << "\n" ;			
	}
	result.close();
}

void LIRSCache::refresh(){
	calls = 0;
	hits = 0;
	migration = 0;
}

void LIRSCache::summary() {
	// print the number of total cache calls, hits, and data migration size
}