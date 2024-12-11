#include "Sorter.h"
#include "defs.h"
#include "tree.h"
#include <queue>

// Method definitions for Sorter
Sorter::Sorter() {
    current_alloc = Alloc::create();
    input_size = 1;
}

void Sorter::add_record(Row *record) {
    TRACE (TRACE_VAL);
    if (current_alloc->can_write(sizeof(Row))) {
        // std::cout << "Record string: " << record->to_string() << "\n";
        // If the current run has space, write the new record to it
        current_alloc->write(static_cast<void*>(record), sizeof(Row));
        return;
    }
    // TODO(): We can use the current_alloc for the subsequent run?
    current_alloc = std::move(sort_current_run());
    if (is_cache_filled()) {
        // Cache is full. Spill to memory
        current_alloc->flush();        
    } else {
        cached_allocs.push_back(current_alloc);
    }
    all_allocs.push_back(std::move(current_alloc));
    current_alloc = Alloc::create();
    current_alloc->write(static_cast<void*>(record), sizeof(Row));
    input_size++;
}

Row& Sorter::get_next_record() {
    TRACE (TRACE_VAL);
    return output_node->read_next();
}

void Sorter::sort_contents() {
    TRACE (TRACE_VAL);
    if (current_alloc->get_size()) {
        current_alloc = std::move(sort_current_run());
        // cached_allocs.push_back(current_alloc);
        all_allocs.push_back(current_alloc);
    }
    if (all_allocs.size() == 1) {
        // All the rows fit in a single cache run
        output_node = std::make_shared<ReaderNode>(current_alloc);
        return;
    }
    // if (all_allocs.size() <= CACHE_SIZE/Alloc::PAGE_SIZE) {
    //     // Internal merge sort the contents
    //     // sort_current_run();
    //     return;
    // }
    // Create merge plan
    output_node = std::move(plan());
    if (output_node->is_internal_node()) {
        auto merge_node = std::static_pointer_cast<MergeNode>(output_node);
        merge_node->execute();
    }
}

bool Sorter::is_cache_filled() {
    size_t capacity = CACHE_SIZE/Alloc::PAGE_SIZE;
    return cached_allocs.size() == capacity;
}

std::shared_ptr<Alloc> Sorter::sort_current_run() {
    TRACE (TRACE_VAL);
    std::shared_ptr<Alloc> output = Alloc::create_alloc(current_alloc->get_size());
    std::vector<std::shared_ptr<SingleElementRun>> inputs;
    for (size_t offset=0; offset < current_alloc->get_size(); offset += sizeof(Row)) {
        auto ptr = std::make_shared<SingleElementRun>(current_alloc->read_record(offset));
        inputs.push_back(std::move(ptr));
    }

    TournamentTree<SingleElementRun> tree {inputs};
    auto inf_record = Row::inf();
    while (true) {
        // TODO(): Reduce memory copies while moving records
        auto top_record = tree.pop();
        if (top_record == inf_record) {
            // Merge is complete when the topmost record is invalid
            break;
        }
        
        // std::cout << "Writing record to output: " << top_record.to_string() << "\n";
        // Write the sorted record
        output->write((void*)(&top_record), sizeof(Row));
    }
    return output;
}

std::shared_ptr<MergeNode> Sorter::plan() {
    TRACE (true);
    // TODO(): Only the initial fan-in needs to be calculated. All other fan-ins should be same as the maximum (merge optimization)
    uint32_t F_final = F; // Final merge fan-in
    size_t W = input_size;
    // Merge smaller-sized runs first
    auto cmp = [](const std::shared_ptr<SortNode> &n1, const std::shared_ptr<SortNode> &n2) {
        return n1->get_size() < n2->get_size();
    };
    // std::cout << "Number of allocs: " << all_allocs.size() << "\n";
    std::priority_queue<std::shared_ptr<SortNode>, std::vector<std::shared_ptr<SortNode>>, decltype(cmp)> nodes(cmp);
    for (auto& alloc: all_allocs) {
        std::shared_ptr<SortNode> node = std::make_shared<ReaderNode>(alloc);
        nodes.push(node);
    }
    while (W > F_final) {
        size_t F_current = (W - F_final - 1) % (F_final - 1);
        std::vector<std::shared_ptr<SortNode>> selected_nodes;
        for (int i=0; i<F_current; i++) {
            selected_nodes.push_back(nodes.top());
            nodes.pop();
        }
        std::shared_ptr<SortNode> new_merge_node = std::make_shared<MergeNode>(selected_nodes);
        nodes.push(new_merge_node);
    }
    std::vector<std::shared_ptr<SortNode>> remaining_nodes;
    while (!nodes.empty()) {
        remaining_nodes.push_back(nodes.top());
        nodes.pop();
    }

    std::shared_ptr<MergeNode> root_node = std::make_shared<MergeNode>(remaining_nodes);
    return root_node;
}

// Method definitions for MergeNode
MergeNode::MergeNode(std::vector<std::shared_ptr<SortNode>> &input_nodes) {
    this->inputs = std::move(input_nodes);
    size = 0;
}

size_t MergeNode::get_size() {
    return size;
}

void MergeNode::execute() {
    TRACE (true);
    size_t total_size = 0ll;
    
    for (auto& input_node: inputs) {
        if (input_node->is_internal_node()) {
            // Recursively execute all children that are merge nodes
            auto input_merge_node = std::static_pointer_cast<MergeNode>(input_node);
            input_merge_node->execute();
        }
        total_size += input_node->get_size();
    }
    size = total_size;
    // Setup memory for output of this run
    output_alloc = Alloc::create_alloc(total_size);
    // Create tournament tree
    TournamentTree<SortNode> tree {inputs};

    auto inf_record = Row::inf();
    while (true) {
        // TODO(): Reduce memory copies while moving records
        auto top_record = tree.pop();
        if (top_record == inf_record) {
            // Merge is complete when the topmost record is invalid
            break;
        }
        
        // Write the sorted record
        output_alloc->write((void*)(&top_record), sizeof(Row));
    }

    read_offset = 0ll;
}

// void MergeNode::execute() {
//     size_t total_size = 0ll;
    
//     for (auto& input_node: inputs) {
//         if (input_node->is_internal_node()) {
//             // Recursively execute all children that are merge nodes
//             auto input_merge_node = std::static_pointer_cast<MergeNode>(input_node);
//             input_merge_node->execute();
//         }
//         total_size += input_node->get_size();
//     }
//     // Setup memory for output of this run
//     output_alloc = Alloc::create_alloc(total_size);
//     // Create tournament tree
//     init_tournament_tree();

//     auto inf_record = DataRecord::inf();
//     while (true) {
//         /**
//          * - Pop from the tree
//          * - Write to a memory page. If current memory page is full, flush and create a new one
//          * - Perform leaf-to-root pass
//          */
//         auto top_node = tournament_tree[0];
//         if (top_node.record == inf_record) {
//             // Merge is complete when the topmost record is invalid
//             break;
//         }
//         leaf_to_root_pass(top_node.index);
//         // Write the sorted record
//         output_alloc->write((void*)(&top_node.record), sizeof(DataRecord));
//         num_records++;
//     }

//     read_offset = 0ll;
// }

// void MergeNode::init_tournament_tree() {
//     size_t input_size = inputs.size();
//     uint32_t closest_power_of_2 = 1;
//     while (closest_power_of_2 < input_size) {
//         closest_power_of_2 *= 2;
//     }
//     tournament_tree.resize(closest_power_of_2);
//     auto val = tree_init_helper(0);
//     tournament_tree[0].index = val.second;
//     tournament_tree[0].record = std::move(val.first);
// }

// TODO(): Make this generic so that it can be used by internal sort as well
// std::pair<DataRecord, uint32_t> MergeNode::tree_init_helper(uint32_t i) {
//     size_t size = tournament_tree.size();
    
//     if (i >= size) {
//         // Called from a leaf node, return a record from the corresponding sorted run
//         uint32_t run_idx = i-size;
//         if (run_idx < inputs.size()) {
//             return std::make_pair<>(inputs[run_idx]->read_next(), run_idx);
//         } else {
//             return std::make_pair<>(DataRecord::inf(), run_idx);
//         }
//     } else {
//         /**
//          * Called from an internal node. Perform the following steps:
//          * - Recursively call helper method for i1=i*2 and i2=i*2+1
//          * - Compare the records obtained from recursive calls and store the larger record (loser) in the current node
//          * - Return the winner to the parent node
//          */
//         uint32_t i1 = 2*i;
//         uint32_t i2 = 2*i + 1;
//         auto val1 = tree_init_helper(i1);
//         auto val2 = tree_init_helper(i2);
//         auto compare_val = (val1.first < val2.first);   // Compare the data records
//         if (compare_val < 0) {
//             tournament_tree[i].record = std::move(val1.first);
//             tournament_tree[i].index = val1.second;
//             return val2;
//         } else {
//             tournament_tree[i].record = std::move(val2.first);
//             tournament_tree[i].index = val2.second;
//             return val1;
//         }
//     }
// }

// void MergeNode::leaf_to_root_pass(uint32_t run_idx) {
//     uint32_t idx = (tournament_tree.size() + run_idx)/2;
//     auto new_record = inputs[run_idx]->read_next();
//     TournamentTreeNode cur_node {new_record, run_idx};
//     while (idx > 0) {
//         /**
//          * Compare cur_node and tournament_tree[idx]
//          * Store the loser
//          * set cur_node to 
//          */
//         if (tournament_tree[idx].record < cur_node.record) {
//             std::swap(tournament_tree[idx], cur_node);
//         }
//         idx /= 2;
//     }
//     tournament_tree[idx] = std::move(cur_node);
// }

Row& MergeNode::read_next() {
    Row& ret_val = *(output_alloc->read_record(read_offset));
    read_offset += sizeof(Row);
    return ret_val;
}

// Method definitions for ReaderNode
ReaderNode::ReaderNode(std::shared_ptr<Alloc> &input): input(input), read_offset(0ll) {
    size = input->get_size();
    input->prepare_for_read();
}

Row& ReaderNode::read_next() {
    Row& ret_val = *(input->read_record(read_offset));
    read_offset += sizeof(Row);
    // std::cout << "Reader node read row: " << ret_val.to_string() << "\n";
    return ret_val;
}