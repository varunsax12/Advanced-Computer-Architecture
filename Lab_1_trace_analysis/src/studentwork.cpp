// -------------------------------------------------- //
// Implement all your code for this lab in this file. //
// This will be the only C++ file you will submit.    //
// -------------------------------------------------- //

// studentwork.cpp
// Analyzes a record in a CPU trace file.

#include "trace.h"
#include <assert.h>

// Added to allow stderr to be reported for unknown OP types
#include <stdio.h>
#include <iostream>

// Added libraries for handling the unique pc tracking
#include <set>

// You may include any other standard C or C++ headers you need here,
// e.g. #include <vector> or #include <algorithm>.
// Make sure this compiles on the reference machine!

// ----------------------------------------------------------------- //
// Do not modify the definitions of the four global variables below. //
// You may add more global variables if you need them.               //
// ----------------------------------------------------------------- //

/**
 * Total number of instructions executed.
 *
 * This variable is updated automatically by the code in sim.cpp.
 * You should not modify it, but you may find it useful for debugging,
 * e.g. as a unique identifier for a record in a trace file.
 */
uint64_t stat_num_inst = 0;

/**
 * Array of number of instructions executed by op type.
 *
 * For instance, stat_optype_dyn[OP_ALU] should be the number of times an ALU
 * instruction was executed in the trace.
 *
 * You must update this on every call to analyze_trace_record().
 */
uint64_t stat_optype_dyn[NUM_OP_TYPES] = {0};

/**
 * Total number of CPU cycles.
 *
 * You must update this on every call to analyze_trace_record().
 */
uint64_t stat_num_cycle = 0;

/**
 * Total number of unique instructions executed.
 *
 * Instructions with the same address should only be counted once.
 *
 * You must update this on every call to analyze_trace_record().
 */
uint64_t stat_unique_pc = 0;

// ------------------------------------------------------------------------- //
// You must implement the body of the analyze_trace_record() function below. //
// Do not modify its return type or argument type.                           //
// You may add helper functions if you need them.                            //
// ------------------------------------------------------------------------- //

/**
 * Updates the global variables stat_num_cycle, stat_optype_dyn, and
 * stat_unique_pc according to the given trace record.
 *
 * You must write code to implement this function.
 * Make sure you DO NOT update stat_num_inst; it is updated by the code in
 * sim.cpp.
 *
 * @param t the trace record to process. Refer to the trace.h header file for
 * details on the TraceRec type.
 */


// Creating global set to track the unique entries
std::set<uint64_t> PC_unique;

void analyze_trace_record(TraceRec *t) {
    assert(t);

    // TODO: Task 1: Quantify the mix of the dynamic instruction stream.
    // Update stat_optype_dyn according to the trace record t.

    // Checking for the different OP types as per OpType defined in trace.h
    // Incrementing the stat of the required OP type on the function call.
    switch (t->optype)
    {
        case OP_ALU:
            stat_optype_dyn[OP_ALU]++;
            break;
        case OP_LD:
            stat_optype_dyn[OP_LD]++;
            break;
        case OP_ST:
            stat_optype_dyn[OP_ST]++;
            break;
        case OP_CBR:
            stat_optype_dyn[OP_CBR]++;
            break;
        case OP_OTHER:
            stat_optype_dyn[OP_OTHER]++;
            break;
        default:
            // added additional check to ensure that it only support entries mentioned in the Lab_1.pdf file
            fprintf(stderr, "Error: Invalid trace during stat computation for task 1\n");
    }

    // TODO: Task 2: Estimate the overall CPI using a simple CPI model in which
    // the CPI for each category of instructions is provided.
    // Update stat_num_cycle according to the trace record t.

    switch (t->optype)
    {
        case OP_ALU:
            stat_num_cycle += 1;
            break;
        case OP_LD:
            stat_num_cycle += 2;
            break;
        case OP_ST:
            stat_num_cycle += 2;
            break;
        case OP_CBR:
            stat_num_cycle += 3;
            break;
        case OP_OTHER:
            stat_num_cycle += 1;
            break;
        default:
            // added additional check to ensure that it only support entries mentioned in the Lab_1.pdf file
            fprintf(stderr, "Error: Invalid trace during stat computation for task 2\n");
    }
    // TODO: Task 3: Estimate the instruction footprint by counting the number
    // of unique PCs in the benchmark trace.
    // Update stat_unique_pc according to the trace record t.

    // Logic: Insert the PC address into the global set. Set will automatically maintain the unique listing
    // So that a separate lookup to check uniqueness of each inst_addr is not needed => faster.
    // Finally, simply check the size of set and keep updating the stat_unique_pc in each call so that in the final
    // call to the function, the final value is picked up.

    PC_unique.insert(t->inst_addr);
    stat_unique_pc = PC_unique.size();

    // Make sure you DO NOT update stat_num_inst.
}
