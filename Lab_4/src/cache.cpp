///////////////////////////////////////////////////////////////////////////////
// You will need to modify this file to implement part A and, for extra      //
// credit, parts E and F.                                                    //
///////////////////////////////////////////////////////////////////////////////

// cache.cpp
// Defines the functions used to implement the cache.

#include "cache.h"
#include <stdio.h>
#include <stdlib.h>
// You may add any other #include directives you need here, but make sure they
// compile on the reference machine!

#include <cmath>
#include <utility>
#include <limits>
#include <random>
#include <iostream>
#include <algorithm>

///////////////////////////////////////////////////////////////////////////////
//                    EXTERNALLY DEFINED GLOBAL VARIABLES                    //
///////////////////////////////////////////////////////////////////////////////

/**
 * The current clock cycle number.
 * 
 * This can be used as a timestamp for implementing the LRU replacement policy.
 */
extern uint64_t current_cycle;

/**
 * For static way partitioning, the quota of ways in each set that can be
 * assigned to core 0.
 * 
 * The remaining number of ways is the quota for core 1.
 * 
 * This is used to implement extra credit part E.
 */
extern unsigned int SWP_CORE0_WAYS;

///////////////////////////////////////////////////////////////////////////////
//                           FUNCTION DEFINITIONS                            //
///////////////////////////////////////////////////////////////////////////////

// As described in cache.h, you are free to deviate from the suggested
// implementation as you see fit.

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
                 ReplacementPolicy replacement_policy)
{
    // TODO: Allocate memory to the data structures and initialize the required
    //       fields. (You might want to use calloc() for this.)
    Cache* c = new Cache;
    c->num_ways = associativity;
    c->num_sets = size / (associativity * line_size);
    // reserve required number of sets
    c->cache_sets.reserve(c->num_sets);
    // For each set reserve the lines
    for (unsigned int i = 0; i < c->num_sets; ++i)
    {
        CacheSet cs;
        for (unsigned int j = 0; j < c->num_ways; ++j)
        {
            CacheLine cl;
            cl.valid = false;
            cl.dirty = false;
            cl.tag = 0;
            cl.coreID = 0;
            cl.last_access_time = 0;
            cl.hits = 0;
            cs.cache_lines.push_back(cl);
        }
        // Init the miss counter for set
        cs.misses = 0;
        c->cache_sets.push_back(cs);
    }

    c->replacement_policy = replacement_policy;

    // Add the number of index and tag bits to use
    c->num_index_bits = std::log2(c->num_sets);
    c->num_tag_bits = 64 - c->num_index_bits;

    // Init stats
    c->stat_read_access = 0;
    c->stat_read_miss = 0;
    c->stat_write_access = 0;
    c->stat_write_miss = 0;
    c->stat_dirty_evicts = 0;

    return c;
}

/*
* Function to get index and tag bits
 * @param c The cache to access.
 * @param line_addr The address of the cache line to access (in units of the
 *                  cache line size, i.e., excluding the line offset bits).
 * @return pair of index and tag
*/
std::pair<uint64_t, uint64_t> get_index_tag_bits(Cache* c, uint64_t line_addr)
{
    uint64_t index = 0;
    uint64_t tag = 0;
    uint64_t mask = std::numeric_limits<uint64_t>::max();
    mask = mask << c->num_index_bits;
    index = line_addr & ~mask;
    tag = line_addr >> c->num_index_bits;
    return std::make_pair(index, tag);
}

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
                         unsigned int core_id)
{
    // TODO: Return HIT if the access hits in the cache, and MISS otherwise.
    // Get the index value
    uint64_t index = 0;
    uint64_t tag = 0;
    std::pair<uint64_t, uint64_t> indexTagPair = get_index_tag_bits(c, line_addr);
    index = indexTagPair.first;
    tag = indexTagPair.second;
    // Check if tag in set
    int lineIndex = -1;
    // check if line has tag
    for (unsigned int i = 0; i < c->cache_sets[index].cache_lines.size(); ++i)
    {
        if (c->cache_sets[index].cache_lines[i].valid == true
            && c->cache_sets[index].cache_lines[i].tag == tag
            && c->cache_sets[index].cache_lines[i].coreID == core_id)
        {
            lineIndex = i;
            break;
        }
    }
    // TODO: If is_write is true, mark the resident line as dirty.
    if (lineIndex != -1)
    {
        c->cache_sets[index].cache_lines[lineIndex].last_access_time = current_cycle;
        if (is_write == true)
        {
            c->cache_sets[index].cache_lines[lineIndex].dirty = true;
        }
    }
    // TODO: Update the appropriate cache statistics.
    if (is_write == false)
    {
        // it is reading
        c->stat_read_access++;
        if (lineIndex == -1)
        {
            c->stat_read_miss++;
            // For Part F, increase the miss counter
            c->cache_sets[index].misses++;
            return MISS;
        }
    }
    else
    {
        // it is writing
        c->stat_write_access++;
        if (lineIndex == -1)
        {
            c->stat_write_miss++;
            // For Part F, increase the miss counter
            c->cache_sets[index].misses++;
            return MISS;
        }
    }
    // For Part F, increase the hits counter
    c->cache_sets[index].cache_lines[lineIndex].hits++;
    return HIT;
}

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
                   unsigned int core_id)
{
    // TODO: Use cache_find_victim() to determine the victim line to evict.
    std::pair<uint64_t, uint64_t> indexTagPair = get_index_tag_bits(c, line_addr);
    unsigned int setIndex = cache_find_victim(c, indexTagPair.first, core_id);
    // TODO: Copy it into a last_evicted_line field in the cache in order to
    //       track writebacks.
    c->last_evicted_line = c->cache_sets[indexTagPair.first].cache_lines[setIndex];
    // TODO: Update the appropriate cache statistics.
    // Update the dirty stat if the evicted line was dirty
    if (c->last_evicted_line.valid == true && c->last_evicted_line.dirty == true)
    {
        c->stat_dirty_evicts++;
    }
    // TODO: Initialize the victim entry with the line to install.
    CacheLine toInstall;
    toInstall.dirty = is_write;
    toInstall.valid = true;
    toInstall.last_access_time = current_cycle;
    toInstall.tag = indexTagPair.second;
    toInstall.coreID = core_id;
    toInstall.hits = 0;

    c->cache_sets[indexTagPair.first].cache_lines[setIndex] = toInstall;
}

/*
* Function to get the victim based on cache partitioning
* 
 * @param c The cache to search.
 * @param set_index The index of the cache set to search.
 * @param core_id The CPU core ID that requested this access.
 * @param partition Calculated partition of the cache set
 * @return The index of the victim way.
*/
unsigned int cache_find_victim_from_partition(Cache* c,unsigned int set_index,
    unsigned int core_id, int partition)
{
    int core0space = 0, core1space = 0;
    // check the quota of each core for the set
    for (auto curr_cache_line : c->cache_sets[set_index].cache_lines)
    {
        if (curr_cache_line.coreID == 0)
        {
            core0space++;
        }
        else
        {
            core1space++;
        }
    }
    int getLRUFrom = -1;
    if (core_id == 0)
    {
        // check if quota exceed for core 1
        if (core1space > (int)c->num_ways - (int)partition)
        {
            getLRUFrom = 1;
        }
        else
        {
            getLRUFrom = 0;
        }
    }
    else
    {
        // check if quota exceed for core 0
        if (core0space > (int)partition)
        {
            getLRUFrom = 0;
        }
        else
        {
            getLRUFrom = 1;
        }
    }
    int lruIndex = -1;
    uint64_t lruTime = std::numeric_limits<uint64_t>::max();
    for (unsigned int i = 0; i < c->num_ways; ++i)
    {
        if (c->cache_sets[set_index].cache_lines[i].coreID == (unsigned int)getLRUFrom &&
            c->cache_sets[set_index].cache_lines[i].last_access_time < lruTime)
        {
            lruIndex = i;
            lruTime = c->cache_sets[set_index].cache_lines[i].last_access_time;

        }
    }
    return lruIndex;
}

/*
* Function to get the victim based on LRU policy
*
 * @param c The cache to search.
 * @param set_index The index of the cache set to search.
 * @param core_id The CPU core ID that requested this access.
 * @return The index of the victim way.
*/
unsigned int cache_find_victim_LRU(Cache* c, unsigned int set_index,
    unsigned int core_id)
{
    unsigned index = -1;
    uint64_t least_cycle_num = std::numeric_limits<uint64_t>::max();
    for (unsigned int i = 0; i < c->num_ways; ++i)
    {
        if (c->cache_sets[set_index].cache_lines[i].last_access_time < least_cycle_num)
        {
            least_cycle_num = c->cache_sets[set_index].cache_lines[i].last_access_time;
            index = i;
        }
    }
    return index;
}

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
                               unsigned int core_id)
{
    // Check for invalid lines in the set
    for (unsigned int i = 0; i < c->num_ways; ++i)
    {
        if (c->cache_sets[set_index].cache_lines[i].valid == false)
        {
            return i;
        }
    }

    // No invalid entry found => use the policy
    if (c->replacement_policy == LRU)
    {
        return cache_find_victim_LRU(c, set_index, core_id);
    }
    else if (c->replacement_policy == RANDOM)
    {
        std::default_random_engine generator;
        std::uniform_int_distribution<int> distribution(0, c->num_ways-1);
        return distribution(generator);
    }
    else if (c->replacement_policy == SWP)
    {
        return cache_find_victim_from_partition(c, set_index, core_id, SWP_CORE0_WAYS);
    }
    else if (c->replacement_policy == DWP)
    {
        // Init the local UMON structure
        // For Core 0
        std::vector<std::pair<uint64_t, unsigned long>> umonCore0;
        // For Core 1
        std::vector<std::pair<uint64_t, unsigned long>> umonCore1;
        for (auto curr_cache_line : c->cache_sets[set_index].cache_lines)
        {
            if (curr_cache_line.coreID == 0)
            {
                umonCore0.push_back(std::make_pair(curr_cache_line.last_access_time, curr_cache_line.hits));
            }
            else
            {
                umonCore1.push_back(std::make_pair(curr_cache_line.last_access_time, curr_cache_line.hits));
            }
        }

        // Check if cache set has lines from both cores or not, if not then simply use LRU policy
        //if (umonCore0.size() == 0 || umonCore1.size() == 0)
        //{
        //    return cache_find_victim_LRU(c, set_index, core_id);
        //}

        // Sort
        std::sort(umonCore0.begin(), umonCore0.end());
        std::reverse(umonCore0.begin(), umonCore0.end());
        std::sort(umonCore1.begin(), umonCore1.end());
        std::reverse(umonCore1.begin(), umonCore1.end());
        // Aim is to maximize utot
        unsigned long utotmax = 0;
        int bestPartition = -1;
        for (int partition = 1; partition < (int)c->num_ways; ++partition)
        {
            unsigned long utot = 0;
            for (int i = 0; i < partition; ++i)
            {
                if (i < (int)umonCore0.size())
                {
                    utot += umonCore0[i].second;
                }
                else
                {
                    utot += 0;
                }
            }
            for (int i = 0; i < (int)c->num_ways - partition; ++i)
            {
                if (i < (int)umonCore1.size())
                {
                    utot += umonCore1[i].second;
                }
                else
                {
                    utot += 0;
                }
            }
            if (utotmax <= utot)
            {
                utot = utotmax;
                bestPartition = partition;
            }
        }
        return cache_find_victim_from_partition(c, set_index, core_id, bestPartition);
    }

    // TODO: Find a victim way in the given cache set according to the cache's
    //       replacement policy.
    // TODO: In part A, implement the LRU and random replacement policies.
    // TODO: In part E, for extra credit, implement static way partitioning.
    // TODO: In part F, for extra credit, implement dynamic way partitioning.
    return -1;
}

/**
 * Print the statistics of the given cache.
 * 
 * This is implemented for you. You must not modify its output format.
 * 
 * @param c The cache to print the statistics of.
 * @param label A label for the cache, which is used as a prefix for each
 *              statistic.
 */
void cache_print_stats(Cache *c, const char *header)
{
    double read_miss_percent = 0.0;
    double write_miss_percent = 0.0;

    if (c->stat_read_access)
    {
        read_miss_percent = 100.0 * (double)(c->stat_read_miss) /
                            (double)(c->stat_read_access);
    }

    if (c->stat_write_access)
    {
        write_miss_percent = 100.0 * (double)(c->stat_write_miss) /
                             (double)(c->stat_write_access);
    }

    printf("\n");
    printf("%s_READ_ACCESS     \t\t : %10llu\n", header, c->stat_read_access);
    printf("%s_WRITE_ACCESS    \t\t : %10llu\n", header, c->stat_write_access);
    printf("%s_READ_MISS       \t\t : %10llu\n", header, c->stat_read_miss);
    printf("%s_WRITE_MISS      \t\t : %10llu\n", header, c->stat_write_miss);
    printf("%s_READ_MISS_PERC  \t\t : %10.3f\n", header, read_miss_percent);
    printf("%s_WRITE_MISS_PERC \t\t : %10.3f\n", header, write_miss_percent);
    printf("%s_DIRTY_EVICTS    \t\t : %10llu\n", header, c->stat_dirty_evicts);
}
