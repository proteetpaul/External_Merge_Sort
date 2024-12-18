#pragma once

#include "Record.h"
#include "Alloc.h"
#include <memory>
#include <iostream>
#include <vector>
#include <boost/align/aligned_allocator.hpp>


// Class representing a run of size 1. This is used for implementing internal sort using tournament trees
class SingleElementRun {
public:
    SingleElementRun(Row* d): d(d) {
        read_called = false;
    } 

    Row read_next() {
        if (!read_called) {
            read_called = true;
            return (*d);
        }
        return Row::inf();
    }
    
private:
    bool read_called;

    Row* d;
    
};

/**
 * Base class to represent a node in the merge tree for external sorting
 */
class SortNode {
public:
    SortNode() = default;

    virtual ~SortNode() = default;

    // Read next record
    virtual Row& read_next() = 0;

    virtual bool is_internal_node() = 0;

    virtual size_t get_size() = 0;
};


/**
 * Class to merge sorted input runs. These are non-leaf nodes in the plan for external merge sort
 */
class MergeNode : public SortNode {
public:
    MergeNode(std::vector<std::shared_ptr<SortNode>> &input_nodes);

    ~MergeNode() = default;

    Row& read_next() override;

    /**
     * Execute the sort plan in a depth-first manner
     */
    void execute();

    bool is_internal_node() override {
        return true;
    }

    size_t get_size() override;

    std::vector<std::shared_ptr<SortNode>> inputs;
private:
    size_t size;

    std::shared_ptr<Alloc> output_alloc;

    size_t read_offset;

    Row inf_row;
};

/**
 * Class to directly read from a sorted run. These are the leaf nodes in the plan for external merge sort
 */
class ReaderNode: public SortNode {
public:
    ReaderNode(std::shared_ptr<Alloc> &input);

    ~ReaderNode() = default;

    Row& read_next() override;

    bool is_internal_node() override {
        return false;
    }

    size_t get_size() override;
private:
    size_t size;

    size_t read_offset;

    std::shared_ptr<Alloc> input {nullptr};

    Row inf_row;
};


/**
 * Class responsible for coordinating the sort operation
 */
class Sorter {
public:
    Sorter();

    /**
     * Add a single record to the Sorter
     */
    void add_record(Row *record);

    /**
     * Return next record in sorted order
     */
    Row& get_next_record();

    /**
     * Sort all records. This is called after all records have been added
     */
    void sort_contents();

private:
    const static size_t CACHE_SIZE = 65536;

    const static size_t F = 65536/4096;     // Cache size / page size

    size_t input_size; // in pages

    std::shared_ptr<Alloc> current_alloc;

    std::vector<std::shared_ptr<Alloc>> all_allocs;

    // Runs currently in CPU cache
    std::vector<std::shared_ptr<Alloc>> cached_allocs;

    std::shared_ptr<SortNode> output_node {nullptr};

    /**
     * Create a merge plan and return the root node
     */
    std::shared_ptr<MergeNode> plan();

    bool is_cache_filled();

    // Perform internal sort on the cache-sized run that is currently being written to. Returns a run containing sorted output
    std::shared_ptr<Alloc> sort_current_run();
};


