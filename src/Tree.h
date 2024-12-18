#pragma once
#include "Record.h"
#include <vector>
#include <iostream>
#include <memory>
#include <boost/align/aligned_allocator.hpp>

/**
 * Struct representing a node in a tournament tree
 */
struct TournamentTreeNode {
    Row record;

    uint32_t index; // run identifier

    TournamentTreeNode(Row record, uint32_t index): record(std::move(record)), index(index) {}

    TournamentTreeNode() = default;
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
    
    Row pop() {
        auto prev_top_node = std::move(top_node);
        leaf_to_root_pass(prev_top_node.index);
        return prev_top_node.record;
    }

private:
    // Store the top node separately
    TournamentTreeNode top_node;

    // Array to hold tournament tree for external merge sort
    // std::vector<TournamentTreeNode, boost::alignment::aligned_allocator<TournamentTreeNode, 64>> tournament_tree;
    std::vector<TournamentTreeNode> tournament_tree;

    // Build the tournament tree from the inputs
    std::vector<std::shared_ptr<ReaderType>> inputs;

    void initialize() {
        size_t input_size = inputs.size();
        uint32_t closest_power_of_2 = 1;
        while (closest_power_of_2 < input_size) {
            closest_power_of_2 *= 2;
        }
        /** 
         * For simplicity, we only resize the tournament tree to the required number of internal nodes
         * In case of leaf nodes, we directly read from the corresponding input runs
        */
        tournament_tree.resize(closest_power_of_2);
        auto res = init_helper(0);
        top_node.index = res.second;
        top_node.record = std::move(res.first);
    }

    // Recursive helper method for building the initial tournament tree
    std::pair<Row, uint32_t> init_helper(uint32_t i) {
        size_t size = tournament_tree.size();
        
        if (i >= size) {
            // Called from a leaf node, return a record from the corresponding sorted run
            uint32_t run_idx = i-size;
            if (run_idx < inputs.size()) {
                return std::make_pair<>(inputs[run_idx]->read_next(), run_idx);
            } else {
                return std::make_pair<>(Row::inf(), run_idx);
            }
        } else {
            /**
             * Called from an internal node. Perform the following steps:
             * - Recursively call helper method for i1=i*2 and i2=i*2+1
             * - Compare the records obtained from recursive calls and store the larger record (loser) in the current node
             * - Return the winner to the parent node
             */
            uint32_t i1 = 2*i + 1;
            uint32_t i2 = 2*i + 2;
            auto val1 = init_helper(i1);
            auto val2 = init_helper(i2);
            auto compare_res = (val1.first < val2.first);   // Compare the data records
            if (compare_res) {
                tournament_tree[i].record = std::move(val2.first);
                tournament_tree[i].index = val2.second;
                return val1;
            } else {
                tournament_tree[i].record = std::move(val1.first);
                tournament_tree[i].index = val1.second;
                return val2;
            }
        }
    }

    // Performs leaf-to-root pass in tournament tree
    void leaf_to_root_pass(uint32_t run_idx) {
        /**
         * Leaf node corresponding to index: (n + run_idx), where n -> size of tournament tree
         * For a leaf node i, its parent will be (i-1)/2
         */
        uint32_t idx = (tournament_tree.size() + run_idx - 1)/2;
        auto new_record = inputs[run_idx]->read_next();
        TournamentTreeNode cur_node {new_record, run_idx};
        while (true) {
            /**
             * Compare cur_node and tournament_tree[idx]
             * Store the loser at position idx
             * Propagate the winner up the tree
             */
            if (tournament_tree[idx].record < cur_node.record) {
                std::swap(tournament_tree[idx], cur_node);
            }
            if (!idx) {
                break;
            }
            idx = (idx-1)/2;
        }
        top_node = std::move(cur_node);
    }
};