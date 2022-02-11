// --------------------------------------------------------------------- //
// For part B, you will need to modify this file.                        //
// You may add any code you need, as long as you correctly implement the //
// three required BPred methods already listed in this file.             //
// --------------------------------------------------------------------- //

// bpred.cpp
// Implements the branch predictor class.

#include "bpred.h"
// Needed to get the BPRED_POLICY variable
#include "pipeline.h"
#include <iostream>
#include <bitset>
#include <map>

/*
* Function to make the upper bits of the address
* apart from the lower 12 bits
* 
* @param address: address to mask
*/
uint64_t mask_12_bits(uint64_t address)
{
    // To make the lower 12 bits we need to create a mask 00000...1111
    // where the 1s are the lower 12 bits
    uint64_t mask = -1; // to create a 11111...1 type of number
    mask = mask << 12;
    mask = ~mask;
    address = mask & address;
    return address;
}

BranchDirection convert_ghsare_state_direction(BPredGharePrediction current)
{
    if (current == STRONGLY_NOTTAKEN || current == WEAKLY_NOTTAKEN)
    {
        return NOT_TAKEN;
    }
    else
    {
        // if it is STRONGLY_TAKEN or WEAKLY_NOTTAKEN
        return TAKEN;
    }
}

/*
* Function to get the pht key from pc
* 
* @param pc: Program counter address
*/
uint64_t BPred::get_pht_key(uint64_t pc)
{
    // Get lower 12 bits of PC
    uint64_t pc_masked = mask_12_bits(pc);
    uint64_t ghr_masked = mask_12_bits(global_history_register);

    // Take the xor
    uint64_t pht_key = pc_masked ^ ghr_masked;
    return pht_key;
}

/*
* Function to access the PHT and get the value
* 
* @param pht_key key to access the PHT
*/
BPredGharePrediction BPred::get_gshare_prediction(uint64_t pht_key)
{
    BPredGharePrediction reset_state = WEAKLY_TAKEN;
    // Check if key in pattern history table
    if (pattern_history_table.find(pht_key) ==
        pattern_history_table.end())
    {
        // key not yet in PHT => BPRED is warming up
        pattern_history_table[pht_key] = reset_state;
        return reset_state;
    }
    else
    {
        // key in PHT => BPRED has warmed up
        return pattern_history_table[pht_key];
    }

    // Adding safety
    return reset_state;
}

/*
* Function to update the PHT table entry
* @param pht_key key to access the PHT
* @param bdirection direction the branch took
*/
void BPred::update_gshare_prediction(uint64_t pht_key, BranchDirection bdirection)
{
    // during update we are sure it exists are during get
    // it would have been reset if the state did not exist
    BPredGharePrediction current_state = pattern_history_table[pht_key];
    BPredGharePrediction new_state;
    if (bdirection == TAKEN)
    {
        // model the FSM
        switch (current_state)
        {
        case STRONGLY_NOTTAKEN:
            new_state = WEAKLY_NOTTAKEN;
            break;
        case WEAKLY_NOTTAKEN:
            new_state = WEAKLY_TAKEN;
            break;
        case WEAKLY_TAKEN:
            new_state = STRONGLY_TAKEN;
            break;
        case STRONGLY_TAKEN:
            new_state = STRONGLY_TAKEN;
            break;
        default:
            new_state = WEAKLY_TAKEN;
            break;
        }
    }
    else
    {
        // not taken
        // model the FSM
        switch (current_state)
        {
        case STRONGLY_NOTTAKEN:
            new_state = STRONGLY_NOTTAKEN;
            break;
        case WEAKLY_NOTTAKEN:
            new_state = STRONGLY_NOTTAKEN;
            break;
        case WEAKLY_TAKEN:
            new_state = WEAKLY_NOTTAKEN;
            break;
        case STRONGLY_TAKEN:
            new_state = WEAKLY_TAKEN;
            break;
        default:
            new_state = WEAKLY_TAKEN;
            break;
        }
    }
    pattern_history_table[pht_key] = new_state;
}

/**
 * Construct a branch predictor with the given policy.
 * 
 * In part B of the lab, you must implement this constructor.
 * 
 * @param policy the policy this branch predictor should use
 */
BPred::BPred(BPredPolicy policy)
{
    // TODO: Initialize member variables here.
    stat_num_branches = 0;
    stat_num_mispred = 0;
    global_history_register = 0;
    // As a reminder, you can declare any additional member variables you need
    // in the BPred class in bpred.h and initialize them here.
}

/**
 * Get a prediction for the branch with the given address.
 * 
 * In part B of the lab, you must implement this method.
 * 
 * @param pc the address (program counter) of the branch to predict
 * @return the prediction for whether the branch is taken or not taken
 */
BranchDirection BPred::predict(uint64_t pc)
{
    // TODO: Return a prediction for whether the branch at address pc will be
    // TAKEN or NOT_TAKEN according to this branch predictor's policy.

    // Note that you do not have to handle the BPRED_PERFECT policy here; this
    // function will not be called for that policy.

    if (BPRED_POLICY == BPRED_ALWAYS_TAKEN)
    {
        return TAKEN; // This is just a placeholder.
    }
    else if (BPRED_POLICY == BPRED_GSHARE)
    {
        // code for gshare here
        uint64_t pht_key = get_pht_key(pc);
        BPredGharePrediction gshare_state = get_gshare_prediction(pht_key);
        return convert_ghsare_state_direction(gshare_state);
    }
    // place holder for default return
    return TAKEN;
}


/**
 * Update the branch predictor statistics (stat_num_branches and
 * stat_num_mispred), as well as any other internal state you may need to
 * update in the branch predictor.
 * 
 * In part B of the lab, you must implement this method.
 * 
 * @param pc the address (program counter) of the branch
 * @param prediction the prediction made by the branch predictor
 * @param resolution the actual outcome of the branch
 */
void BPred::update(uint64_t pc, BranchDirection prediction,
                   BranchDirection resolution)
{
    // TODO: Update the stat_num_branches and stat_num_mispred member variables
    // according to the prediction and resolution of the branch.
    stat_num_branches = sat_increment(stat_num_branches, UINT64_MAX);
    if (prediction != resolution)
    {
        stat_num_mispred = sat_increment(stat_num_mispred, UINT64_MAX);
    }

    if (BPRED_POLICY == BPRED_GSHARE)
    {
        //print_branch_state(pc, resolution, prediction);
        // Update the PHT before the GHR is update else the hash key will change
        // Update the PHT
        uint64_t pht_key = get_pht_key(pc);
        update_gshare_prediction(pht_key, resolution);
        // Update the GHR
        // Left shift the GHR
        global_history_register = global_history_register << 1;
        if (resolution == TAKEN)
        {
            // set the last bit
            global_history_register = global_history_register | 1;
        }
    }
    // TODO: Update any other internal state you may need to keep track of.

    // Note that you do not have to handle the BPRED_PERFECT policy here; this
    // function will not be called for that policy.
}

template<typename K, typename V>
void print_map(std::unordered_map<K, V> const& m)
{
    std::map<uint64_t, BPredGharePrediction> ordered(m.begin(), m.end());
    for (auto const& pair : ordered) {
        std::cout << "{" << (unsigned long)(pair.first) << ": ";
        switch (pair.second)
        {
        case STRONGLY_NOTTAKEN:
            std::cout << "STRONGLY_NOTTAKEN}\n";
            break;
        case WEAKLY_NOTTAKEN:
            std::cout << "WEAKLY_NOTTAKEN}\n";
            break;
        case WEAKLY_TAKEN:
            std::cout << "WEAKLY_TAKEN}\n";
            break;
        case STRONGLY_TAKEN:
            std::cout << "STRONGLY_TAKEN}\n";
            break;
        default:
            break;
        }
    }
}

/*
* Function to print the GHR and PHT
*/
void BPred::print_branch_state(uint64_t pc, BranchDirection resolution, BranchDirection prediction)
{
    std::cout << "Branch direction = " << resolution;
    std::cout << "\nBranch prediction = " << prediction;
    std::cout << "\nCurrent PC                = " << (unsigned long)(pc);
    std::cout << "\nGlobal history register   = ";
    std::cout << (unsigned long)(global_history_register);
    std::cout << "\nPHT Key                   = " << (unsigned long)(get_pht_key(pc));
    std::cout << "\nPattern history table state\n";
    print_map<uint64_t, BPredGharePrediction>(pattern_history_table);
    std::cout << "\n\n";
}
