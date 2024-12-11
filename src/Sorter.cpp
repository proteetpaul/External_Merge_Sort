#include "Sorter.h"
#include "defs.h"
#include "Tree.h"
#include <queue>

// Method definitions for Sorter
Sorter::Sorter() {
    current_alloc = Alloc::create();
    input_size = 1;
}

void Sorter::add_record(Row *record) {
    if (current_alloc->can_write(sizeof(Row))) {
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
    return output_node->read_next();
}

void Sorter::sort_contents() {
    if (current_alloc->get_size()) {
        current_alloc = std::move(sort_current_run());
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
    // TRACE (TRACE_VAL);
    //  std::cout << "Size of current alloc: " << current_alloc->get_size() << "\n";
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

        output->write((void*)(&top_record), sizeof(Row));
    }
    return output;
}

std::shared_ptr<MergeNode> Sorter::plan() {
    TRACE (TRACE_VAL);
    uint32_t F_final = F; // Final merge fan-in
    size_t W = all_allocs.size();
    if (W <= F) {
        // Internal merge sort
        std::vector<std::shared_ptr<SortNode>> input_nodes;
        for (auto& alloc: all_allocs) {
            std::shared_ptr<SortNode> node = std::make_shared<ReaderNode>(alloc);
            input_nodes.push_back(node);
        }
        return std::make_shared<MergeNode>(input_nodes);
    }
    // Merge smaller-sized runs first
    auto cmp = [](const std::shared_ptr<SortNode> &n1, const std::shared_ptr<SortNode> &n2) {
        return n1->get_size() > n2->get_size();
    };
    // std::cout << "Number of allocs: " << all_allocs.size() << "\n";
    std::priority_queue<std::shared_ptr<SortNode>, std::vector<std::shared_ptr<SortNode>>, decltype(cmp)> nodes(cmp);
    for (auto& alloc: all_allocs) {
        std::shared_ptr<SortNode> node = std::make_shared<ReaderNode>(alloc);
        nodes.push(node);
    }

    uint32_t initial_fan_in = (W - F_final - 1) % (F_final - 1) + 2;
    bool first_merge = true;
    while (nodes.size() > 1) {
        // Compute current fan-in
        uint32_t fan_in = (first_merge)? initial_fan_in: F_final;
        std::vector<std::shared_ptr<SortNode>> selected_nodes;
        for (uint32_t i=0; i<fan_in; i++) {
            selected_nodes.push_back(nodes.top());
            nodes.pop();
        }
        std::shared_ptr<SortNode> new_merge_node = std::make_shared<MergeNode>(selected_nodes);
        nodes.push(new_merge_node);
        first_merge = false;
    }
    std::shared_ptr<MergeNode> root_node = std::static_pointer_cast<MergeNode>(nodes.top());
    return root_node;
}


// Method definitions for MergeNode
MergeNode::MergeNode(std::vector<std::shared_ptr<SortNode>> &input_nodes) 
        : SortNode() {
    this->inputs = std::move(input_nodes);
    inf_row = std::move(Row::inf());
    size = 0;
    for (auto& input: inputs) {
        size += input->get_size();
    }
}

size_t MergeNode::get_size() {
    return size;
}

void MergeNode::execute() {
    for (auto& input_node: inputs) {
        if (input_node->is_internal_node()) {
            // Recursively execute all children that are merge nodes
            auto input_merge_node = std::static_pointer_cast<MergeNode>(input_node);
            input_merge_node->execute();
        }
    }
    // Setup memory for output of this run
    output_alloc = Alloc::create_alloc(size);
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


Row& MergeNode::read_next() {
    if (read_offset >= size) return inf_row;

    Row& ret_val = *(output_alloc->read_record(read_offset));
    read_offset += sizeof(Row);
    return ret_val;
}


// Method definitions for ReaderNode
ReaderNode::ReaderNode(std::shared_ptr<Alloc> &input): 
        SortNode(), read_offset(0ll), input(input) {
    size = input->get_size();
    input->prepare_for_read();
    inf_row = std::move(Row::inf());
}

Row& ReaderNode::read_next() {
    if (read_offset >= size) return inf_row;

    Row& ret_val = *(input->read_record(read_offset));
    read_offset += sizeof(Row);
    return ret_val;
}

size_t ReaderNode::get_size(){
    return size;
};