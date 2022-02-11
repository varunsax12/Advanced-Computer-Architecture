///////////////////////////////////////////////////////////////////////////////
// You will need to modify this file to implement parts B and C.             //
///////////////////////////////////////////////////////////////////////////////

// dram.cpp
// Defines the functions used to implement DRAM.

#include "dram.h"
#include <stdio.h>
#include <stdlib.h>
// You may add any other #include directives you need here, but make sure they
// compile on the reference machine!

#include <math.h>
#include <limits>
#include <iostream>

///////////////////////////////////////////////////////////////////////////////
//                                 CONSTANTS                                 //
///////////////////////////////////////////////////////////////////////////////

/** The fixed latency of a DRAM access assumed in part B, in cycles. */
#define DELAY_SIM_MODE_B 100

/** The DRAM activation latency (ACT), in cycles. (Also known as RAS.) */
#define DELAY_ACT 45

/** The DRAM column selection latency (CAS), in cycles. */
#define DELAY_CAS 45

/** The DRAM precharge latency (PRE), in cycles. */
#define DELAY_PRE 45

/**
 * The DRAM bus latency, in cycles.
 * 
 * This is how long it takes for the DRAM to transmit the data via the bus. It
 * is incurred on every DRAM access. (In part B, assume that this is included
 * in the fixed DRAM latency DELAY_SIM_MODE_B.)
 */
#define DELAY_BUS 10

/** The row buffer size, in bytes. */
#define ROW_BUFFER_SIZE 1024

/** The number of banks in the DRAM module. */
#define NUM_BANKS 16

///////////////////////////////////////////////////////////////////////////////
//                    EXTERNALLY DEFINED GLOBAL VARIABLES                    //
///////////////////////////////////////////////////////////////////////////////

/**
 * The current mode under which the simulation is running, corresponding to
 * which part of the lab is being evaluated.
 */
extern Mode SIM_MODE;

/** The number of bytes in a cache line. */
extern uint64_t CACHE_LINESIZE;

/** Which page policy the DRAM should use. */
extern DRAMPolicy DRAM_PAGE_POLICY;

///////////////////////////////////////////////////////////////////////////////
//                           FUNCTION DEFINITIONS                            //
///////////////////////////////////////////////////////////////////////////////

// As described in dram.h, you are free to deviate from the suggested
// implementation as you see fit.

// The only restriction is that you must not remove dram_print_stats() or
// modify its output format, since its output will be used for grading.

/**
 * Allocate and initialize a DRAM module.
 * 
 * This is intended to be implemented in part B.
 * 
 * @return A pointer to the DRAM module.
 */
DRAM *dram_new()
{
    // TODO: Allocate memory to the data structures and initialize the required
    //       fields. (You might want to use calloc() for this.)
    DRAM* dram = new DRAM;

    // init the stats
    dram->stat_read_access = 0;
    dram->stat_read_delay = 0;
    dram->stat_write_access = 0;
    dram->stat_write_delay = 0;

    // init the row buffer array
    // NOTE: Recitation slide mentions always use 16 banks
    dram->row_buffer_array.reserve(NUM_BANKS);

    for (unsigned int i = 0; i < NUM_BANKS; ++i)
    {
        dram->row_buffer_array[i].valid = false;
    }

    dram->num_bank_bits = log2(NUM_BANKS);
    dram->num_tag_bits = 64 - dram->num_bank_bits;

    return dram;
}

/**
 * Access the DRAM at the given cache line address.
 * 
 * Return the delay in cycles incurred by this DRAM access. Also update the
 * DRAM statistics accordingly.
 * 
 * Note that the address is given in units of the cache line size!
 * 
 * This is intended to be implemented in parts B and C. In parts C through F,
 * you may delegate logic to the dram_access_mode_CDEF() functions.
 * 
 * @param dram The DRAM module to access.
 * @param line_addr The address of the cache line to access (in units of the
 *                  cache line size).
 * @param is_dram_write Whether this access writes to DRAM.
 * @return The delay in cycles incurred by this DRAM access.
 */
uint64_t dram_access(DRAM *dram, uint64_t line_addr, bool is_dram_write)
{
    uint64_t delay = 0;
    if (SIM_MODE == SIM_MODE_B)
    {
        // TODO: Update the appropriate DRAM statistics.
        if (is_dram_write)
        {
            dram->stat_write_access++;
            dram->stat_write_delay += 100;
        }
        else
        {
            dram->stat_read_access++;
            dram->stat_read_delay += 100;
        }
        delay += 100;
    }
    // TODO: Call the dram_access_mode_CDEF() function as needed.
    else
    {
        uint64_t current_delay = dram_access_mode_CDEF(dram, line_addr, is_dram_write);
        if (is_dram_write)
        {
            dram->stat_write_access++;
            dram->stat_write_delay += current_delay;
        }
        else
        {
            dram->stat_read_access++;
            dram->stat_read_delay += current_delay;
        }
        delay += current_delay;
    }
    // TODO: Return the delay in cycles incurred by this DRAM access.
    return delay;
}

/**
 * For parts C through F, access the DRAM at the given cache line address.
 * 
 * Return the delay in cycles incurred by this DRAM access. It is intended that
 * the calling function will be responsible for updating DRAM statistics
 * accordingly.
 * 
 * Note that the address is given in units of the cache line size!
 * 
 * This is intended to be implemented in part C.
 * 
 * @param dram The DRAM module to access.
 * @param line_addr The address of the cache line to access (in units of the
 *                  cache line size).
 * @param is_dram_write Whether this access writes to DRAM.
 * @return The delay in cycles incurred by this DRAM access.
 */
uint64_t dram_access_mode_CDEF(DRAM *dram, uint64_t line_addr,
                               bool is_dram_write)
{
    uint64_t delay = 0;
    // Assume a mapping with consecutive lines in the same row and consecutive
    // row buffers in consecutive rows.
    
    // Convert the line address to complete address
    // Get the bits for block offset and shift address by that amount
    line_addr = line_addr << (int)log2(CACHE_LINESIZE);
    // To remove the column and byte in bus bits
    line_addr = line_addr >> (int)log2(ROW_BUFFER_SIZE);

    std::pair<uint64_t, uint64_t> rowBankPair = get_row_bank_bits(dram, line_addr);
    uint64_t row = rowBankPair.first;
    uint64_t bank = rowBankPair.second;

    // Check the access latency
    if (DRAM_PAGE_POLICY == CLOSE_PAGE)
    {
        delay += DELAY_ACT;
        delay += DELAY_CAS;
        delay += DELAY_BUS;

        // Not adding the data transaction of updating row buffer,
        // as it is done each time anyways
    }
    else
    {
        // OPEN PAGE
        // Check row buffer
        if (dram->row_buffer_array[bank].valid == false)
        {
            // row buffer empty
            delay += DELAY_ACT;
            delay += DELAY_CAS;
            delay += DELAY_BUS;

            // Update row buffer
            dram->row_buffer_array[bank].row_id = row;
            dram->row_buffer_array[bank].valid = true;
        }
        else
        {
            // row buffer not empty
            if (dram->row_buffer_array[bank].row_id == row)
            {
                // row buffer hit
                delay += DELAY_CAS;
                delay += DELAY_BUS;
            }
            else
            {
                // row buffer miss
                delay += DELAY_PRE;
                delay += DELAY_ACT;
                delay += DELAY_CAS;
                delay += DELAY_BUS;

                // Update row buffer
                dram->row_buffer_array[bank].row_id = row;
                dram->row_buffer_array[bank].valid = true;
            }

        }
    }

    // TODO: Use this function to track open rows.
    // TODO: Compute the delay based on row buffer hit/miss/empty.

    return delay;
}

/**
 * Print the statistics of the DRAM module.
 * 
 * This is implemented for you. You must not modify its output format.
 * 
 * @param dram The DRAM module to print the statistics of.
 */
void dram_print_stats(DRAM *dram)
{
    double avg_read_delay = 0.0;
    double avg_write_delay = 0.0;

    if (dram->stat_read_access)
    {
        avg_read_delay = (double)(dram->stat_read_delay) /
                         (double)(dram->stat_read_access);
    }

    if (dram->stat_write_access)
    {
        avg_write_delay = (double)(dram->stat_write_delay) /
                          (double)(dram->stat_write_access);
    }

    printf("\n");
    printf("DRAM_READ_ACCESS     \t\t : %10llu\n", dram->stat_read_access);
    printf("DRAM_WRITE_ACCESS    \t\t : %10llu\n", dram->stat_write_access);
    printf("DRAM_READ_DELAY_AVG  \t\t : %10.3f\n", avg_read_delay);
    printf("DRAM_WRITE_DELAY_AVG \t\t : %10.3f\n", avg_write_delay);
}

/*
* Function to get bank and row id bits
 * @param dram The dram to access.
 * @param line_addr The address of the row in dram (after removing the column and bank bits)
 * @return pair of row_id and bank bits
*/
std::pair<uint64_t, uint64_t> get_row_bank_bits(DRAM* dram, uint64_t line_addr)
{
    uint64_t bank = 0;
    uint64_t row = 0;
    uint64_t mask = std::numeric_limits<uint64_t>::max();
    mask = mask << dram->num_bank_bits;
    bank = line_addr & ~mask;
    row = line_addr >> dram->num_bank_bits;
    return std::make_pair(row, bank);
}
