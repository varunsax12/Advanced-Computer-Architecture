///////////////////////////////////////////////////////////////////////////////
// You will need to modify this file to implement part A.                    //
///////////////////////////////////////////////////////////////////////////////

// cache.h
// Contains declarations of data structures and functions used to implement a
// cache.

#ifndef __CACHE_H__
#define __CACHE_H__

#include "types.h"
// You may add any other #include directives you need here, but make sure they
// compile on the reference machine!

#include <vector>
#include <utility>

///////////////////////////////////////////////////////////////////////////////
//                                 CONSTANTS                                 //
///////////////////////////////////////////////////////////////////////////////

/**
 * The maximum allowed number of ways in a cache set.
 * 
 * At runtime, the actual number of ways in each cache set is guaranteed to be
 * less than or equal to this value.
 */
#define MAX_WAYS_PER_CACHE_SET 16

///////////////////////////////////////////////////////////////////////////////
//                              DATA STRUCTURES                              //
///////////////////////////////////////////////////////////////////////////////

// TODO: Define any other data structures you need here.
// Refer to Appendix A for details on data structures you will need here.

/** Whether a cache access is a hit or a miss. */
typedef enum CacheResultEnum
{
    HIT = 1,  // The access hit the cache.
    MISS = 0, // The access missed the cache.
} CacheResult;

/** Possible replacement policies for the cache. */
typedef enum ReplacementPolicyEnum
{
    LRU = 0,    // Evict the least recently used line.
    RANDOM = 1, // Evict a random line.

    /**
     * Evict according to a static way partitioning policy.
     * Part E asks you to implement this policy for extra credit.
     */
     SWP = 2,

     /**
      * Evict according to a dynamic way partitioning policy.
      * Part F asks you to implement this policy for extra credit.
      */
      DWP = 3,
} ReplacementPolicy;

/*
* Defining the structure for a cache line
*/
typedef struct CacheLine
{
    /*
    * Valid bit to denote presence in cahce
    */
    bool valid;

    /*
    * Dirty bit to denote modifications done in line
    */
    bool dirty;

    /*
    * Tag of the cache line
    */
    uint64_t tag;

    /*
    * Core ID to identify the core from which the data is coming
    */
    unsigned int coreID;

    /*
    * Last acces time to track the last access of this variable in number of cycles
    */
    uint64_t last_access_time;

    /*
    * For Part F, track the hit rate for each line
    */
    unsigned long hits;

} CacheLine;

/*
* Defining structure for cache set
*/
typedef struct CacheSet
{
    /* 
    * Defining array of cache lines to denote the different ways
    */
    std::vector<CacheLine> cache_lines;

    /*
    * For Part F, track the number of misses for set
    */
    unsigned long misses;

} CacheSet;

/** A single cache module. */
typedef struct Cache
{
    // TODO: Define any other fields you need here.
    // Refer to Appendix A for details on other fields you will need here.

    /*
    * Number of ways in set
    */
    unsigned int num_ways;

    /*
    * Number of sets in cache
    */
    unsigned int num_sets;

    /*
    * Number of index bits
    */
    unsigned int num_index_bits;

    /*
    * Number of tag bits
    */
    unsigned int num_tag_bits;

    /*
    * Replacement policy to use
    */
    ReplacementPolicy replacement_policy;

    /*
    * Last evicted line (Cache line type) to be passed to the next level of hierarchy for an
    * install if necessay
    */
    CacheLine last_evicted_line;

    /*
    * Cache set holding vector
    */
    std::vector<CacheSet> cache_sets;


    /**
     * The total number of times this cache was accessed for a read.
     * You should initialize this to 0 and update it for every read!
     */
    unsigned long long stat_read_access;

    /**
     * The total number of read accesses that missed this cache.
     * You should initialize this to 0 and update it for every read miss!
     */
    unsigned long long stat_read_miss;

    /**
     * The total number of times this cache was accessed for a write.
     * You should initialize this to 0 and update it for every write!
     */
    unsigned long long stat_write_access;

    /**
     * The total number of write accesses that missed this cache.
     * You should initialize this to 0 and update it for every write miss!
     */
    unsigned long long stat_write_miss;

    /**
     * The total number of times a dirty line was evicted from this cache.
     * You should initialize this to 0 and update it for every dirty eviction!
     */
    unsigned long long stat_dirty_evicts;
} Cache;

///////////////////////////////////////////////////////////////////////////////
//                            FUNCTION PROTOTYPES                            //
///////////////////////////////////////////////////////////////////////////////

// Please note:
// Implementing the following functions as described will be useful in
// completing the lab.

// However, if you would like to deviate from the suggested implementation,
// you are free to do so by adding, removing, or modifying declarations as you
// see fit.

// The only restriction is that you must not remove cache_print_stats() or
// modify its output format, since its output will be used for grading.

/**
 * Allocate and initialize a cache.
 * 
 * This is intended to be implemented in part A.
 *
 * @param size The size of the cache in bytes.
 * @param associativity The associativity of the cache.
 * @param line_size The size of a cache line in bytes.
 * @param replacement_policy The replacement policy of the cache.
 * @return A pointer to the cache.
 */
Cache *cache_new(uint64_t size, uint64_t associativity, uint64_t line_size,
                 ReplacementPolicy replacement_policy);

/**
 * Access the cache at the given address.
 * 
 * Also update the cache statistics accordingly.
 * 
 * This is intended to be implemented in part A.
 * 
 * @param c The cache to access.
 * @param line_addr The address of the cache line to access (in units of the
 *                  cache line size, i.e., excluding the line offset bits).
 * @param is_write Whether this access is a write.
 * @param core_id The CPU core ID that requested this access.
 * @return Whether the cache access was a hit or a miss.
 */
CacheResult cache_access(Cache *c, uint64_t line_addr, bool is_write,
                         unsigned int core_id);

/**
 * Install the cache line with the given address.
 * 
 * Also update the cache statistics accordingly.
 * 
 * This is intended to be implemented in part A.
 * 
 * @param c The cache to install the line into.
 * @param line_addr The address of the cache line to install (in units of the
 *                  cache line size, i.e., excluding the line offset bits).
 * @param is_write Whether this install is triggered by a write.
 * @param core_id The CPU core ID that requested this access.
 */
void cache_install(Cache *c, uint64_t line_addr, bool is_write,
                   unsigned int core_id);

/**
 * Find which way in a given cache set to replace when a new cache line needs
 * to be installed. This should be chosen according to the cache's replacement
 * policy.
 * 
 * The returned victim can be valid (non-empty), in which case the calling
 * function is responsible for evicting the cache line from that victim way.
 * 
 * This is intended to be initially implemented in part A and, for extra
 * credit, extended in parts E and F.
 * 
 * @param c The cache to search.
 * @param set_index The index of the cache set to search.
 * @param core_id The CPU core ID that requested this access.
 * @return The index of the victim way.
 */
unsigned int cache_find_victim(Cache *c, unsigned int set_index,
                               unsigned int core_id);

/**
 * Print the statistics of the given cache.
 * 
 * This is implemented for you. You must not modify its output format.
 * 
 * @param c The cache to print the statistics of.
 * @param label A label for the cache, which is used as a prefix for each
 *              statistic.
 */
void cache_print_stats(Cache *c, const char *label);

/*
* Function to get index and tag bits
 * @param c The cache to access.
 * @param line_addr The address of the cache line to access (in units of the
 *                  cache line size, i.e., excluding the line offset bits).
 * @return pair of index and tag
*/
std::pair<uint64_t, uint64_t> get_index_tag_bits(Cache* c, uint64_t line_addr);

#endif // __CACHE_H__
