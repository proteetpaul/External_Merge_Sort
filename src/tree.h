#pragma once
#include "Record.h"
#include <vector>
#include <memory>
#include <boost/align/aligned_allocator.hpp>

/**
 * Struct representing a node in a tournament tree
 */
struct TournamentTreeNode {
    DataRecord record;

    uint32_t index; // run identifier

    TournamentTreeNode(DataRecord record, uint32_t index): record(std::move(record)), index(index) {}
};

/**
 * Class representing a tree-of-losers priority queue. Given a set of inputs, this class is responsible for building 
 * the tournament tree and performing merge sort via leaf-to-root passes. We templatize this class since we use the same
 * implementation for both external and internal sort
 */
template<typename ReaderType>
class TournamentTree {
public:
    TournamentTree(std::vector<std::shared_ptr<ReaderType>> &inputs): inputs(inputs) {
        initialize();
    }
    
    DataRecord pop() {
        auto top_node = tournament_tree[0];
        leaf_to_root_pass(top_node.index);
        return top_node.record;
    }

private:
    // Array to hold tournament tree for external merge sort
    std::vector<TournamentTreeNode, boost::alignment::aligned_allocator<TournamentTreeNode, 64>> tournament_tree;

    // Build the tournament tree from the inputs
    std::vector<std::shared_ptr<ReaderType>> inputs;

    void initialize() {
        size_t input_size = inputs.size();
        uint32_t closest_power_of_2 = 1;
        while (closest_power_of_2 < input_size) {
            closest_power_of_2 *= 2;
        }
        tournament_tree.resize(closest_power_of_2);
        auto val = init_helper(0);
        tournament_tree[0].index = val.second;
        tournament_tree[0].record = std::move(val.first);
    }

    // Recursive helper method for building the initial tournament tree
    std::pair<DataRecord, uint32_t> init_helper(uint32_t i) {
        size_t size = tournament_tree.size();
        
        if (i >= size) {
            // Called from a leaf node, return a record from the corresponding sorted run
            uint32_t run_idx = i-size;
            if (run_idx < inputs.size()) {
                return std::make_pair<>(inputs[run_idx]->read_next(), run_idx);
            } else {
                return std::make_pair<>(DataRecord::inf(), run_idx);
            }
        } else {
            /**
             * Called from an internal node. Perform the following steps:
             * - Recursively call helper method for i1=i*2 and i2=i*2+1
             * - Compare the records obtained from recursive calls and store the larger record (loser) in the current node
             * - Return the winner to the parent node
             */
            uint32_t i1 = 2*i;
            uint32_t i2 = 2*i + 1;
            auto val1 = init_helper(i1);
            auto val2 = init_helper(i2);
            auto compare_val = (val1.first < val2.first);   // Compare the data records
            if (compare_val < 0) {
                tournament_tree[i].record = std::move(val1.first);
                tournament_tree[i].index = val1.second;
                return val2;
            } else {
                tournament_tree[i].record = std::move(val2.first);
                tournament_tree[i].index = val2.second;
                return val1;
            }
        }
    }

    // Performs leaf-to-root pass in tournament tree
    void leaf_to_root_pass(uint32_t run_idx) {
        uint32_t idx = (tournament_tree.size() + run_idx)/2;
        auto new_record = inputs[run_idx]->read_next();
        TournamentTreeNode cur_node {new_record, run_idx};
        while (idx > 0) {
            /**
             * Compare cur_node and tournament_tree[idx]
             * Store the loser at position idx
             * Propagate the winner up the tree
             */
            if (tournament_tree[idx].record < cur_node.record) {
                std::swap(tournament_tree[idx], cur_node);
            }
            idx /= 2;
        }
        tournament_tree[idx] = std::move(cur_node);
    }
};