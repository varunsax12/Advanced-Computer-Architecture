// ---------------------------------------------------------------------- //
// For part B, you may need to modify this file.                          //
// You may add any declarations you may need, but do not modify or remove //
// the existing declarations, except for adding new member variables and  //
// methods to the BPred class.                                            //
// ---------------------------------------------------------------------- //

// bpred.h
// Declares the branch predictor class, as well as enums and utility functions
// related to it.

#ifndef _BPRED_H_
#define _BPRED_H_

#include <inttypes.h>
#include <unordered_map>

/**
 * The possible branch prediction policies the simulator can use.
 * 
 * DO NOT MODIFY OR REMOVE THIS ENUM.
 */
typedef enum BPredPolicyEnum
{
    BPRED_PERFECT,      // The branch predictor is (magically) always correct.
    BPRED_ALWAYS_TAKEN, // The branch predictor always predicts a branch taken.
    BPRED_GSHARE,       // The branch predictor uses the Gshare algorithm.
    NUM_BPRED_POLICIES
} BPredPolicy;

/**
 * Whether a branch is taken or not taken.
 * 
 * DO NOT MODIFY OR REMOVE THIS ENUM.
 */
typedef enum BranchDirectionEnum
{
    NOT_TAKEN = 0, // The branch is not taken.
    TAKEN = 1      // The branch is taken.
} BranchDirection;

/*
* Current state of the gshare PHT
*/
typedef enum BPredGharePredictionEnum
{
    STRONGLY_TAKEN,
    WEAKLY_TAKEN,
    WEAKLY_NOTTAKEN,
    STRONGLY_NOTTAKEN
} BPredGharePrediction;

/**
 * A branch predictor.
 * 
 * Add any additional member variables or methods you need to this class, but
 * DO NOT MODIFY OR REMOVE ANY OF THE EXISTING DECLARATIONS.
 */
class BPred
{
private:
    /** The policy this branch predictor uses. */
    BPredPolicy policy;

public:
    /** The total number of branches this branch predictor has seen. */
    uint64_t stat_num_branches;
    /** The number of branches this branch predictor has mispredicted. */
    uint64_t stat_num_mispred;

    /* Global history register */
    uint64_t global_history_register;

    /* Patter history table */
    std::unordered_map<uint64_t, BPredGharePrediction> pattern_history_table;

    /**
     * Construct a branch predictor with the given policy.
     * 
     * In part B of the lab, you must implement this constructor.
     * 
     * @param policy the policy this branch predictor should use
     */
    BPred(BPredPolicy policy);

    /**
     * Get a prediction for the branch with the given address.
     * 
     * In part B of the lab, you must implement this method.
     * 
     * @param pc the address (program counter) of the branch to predict
     * @return the prediction for whether the branch is taken or not taken
     */
    BranchDirection predict(uint64_t pc);

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
    void update(uint64_t pc, BranchDirection prediction,
                BranchDirection resolution);

    /*
    * Function to get the PHT key from the pc
    */
    uint64_t get_pht_key(uint64_t pc);

    /*
    * Function to access the PHT and get the value
    */
    BPredGharePrediction get_gshare_prediction(uint64_t pht_key);

    /*
    * Function to update the PHT table entry
    */
    void update_gshare_prediction(uint64_t pht_key, BranchDirection bdirection);

    /*
    * Function to print the GHR and PHT
    */
    void print_branch_state(uint64_t pht_key, BranchDirection resolution, BranchDirection prediction);
};

/**
 * Saturating increment: a utility function to increment a value by 1, stopping
 * at a given maximum value.
 * 
 * You may find this function useful in your branch predictor implementation,
 * but you are not required to use it.
 * 
 * @param x the value to increment
 * @param max the maximum value to increment to
 * @return the incremented value if x < max, or x otherwise
 */
static inline uint32_t sat_increment(uint64_t x, uint64_t max)
{
    if (x < max)
    {
        return x + 1;
    }
    else
    {
        return x;
    }
}

/**
 * Saturating decrement: a utility function to decrement a value by 1, stopping
 * at 0.
 * 
 * You may find this function useful in your branch predictor implementation,
 * but you are not required to use it.
 * 
 * @param x the value to decrement
 * @return the decremented value if x > 0, or x otherwise
 */
static inline uint32_t sat_decrement(uint64_t x)
{
    if (x > 0)
    {
        return x - 1;
    }
    else
    {
        return x;
    }
}

#endif
