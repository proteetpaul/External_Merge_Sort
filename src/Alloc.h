#pragma once
#include "defs.h"
#include <iostream>
#include <cstdint>
#include <memory>
#include <unistd.h>
#include <cstring>
#include <emmintrin.h>
#include <sys/mman.h>

/**
 * Class to represent a cache-sized run in memory
 */
class Alloc {
public:
    Alloc() {
        write_offset = 0;
    }

    static std::shared_ptr<Alloc> create(size_t size=PAGE_SIZE) {
        // make size a multiple of 64 bytes to avoid cache line split
        size += (CACHE_LINE_SIZE - size%CACHE_LINE_SIZE);
        auto alloc_ptr = std::make_shared<Alloc>();
        alloc_ptr->capacity = size;
        // Use mmap so that each memory allocation is done on a fresh page
        alloc_ptr->start_addr = mmap64(nullptr, size, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
        if (alloc_ptr->start_addr == MAP_FAILED) {
            return nullptr;
        }
        return alloc_ptr;
    }

    ~Alloc() {
        munmap(start_addr, capacity);
    }

    inline void prepare_for_read() {
        TRACE (TRACE_VAL);
        read_offset = 0ll;
    }

    inline Row* read_record(size_t offset) {
        return reinterpret_cast<Row*>(start_addr + offset);
    }

    // Explicitly flush the cache lines to memory
    inline void flush() {
        void *cur_ptr = start_addr;
        for (size_t offset = 0; offset < capacity; offset += CACHE_LINE_SIZE) {
            _mm_clflush((char *)start_addr + offset);
        }
    }

    inline bool can_write(size_t bytes) {
        return (capacity - write_offset >= bytes);
    }

    inline void write(const void *ptr, size_t bytes) {
        memcpy(start_addr + write_offset, ptr, bytes);
        write_offset += bytes;
    }

    inline size_t get_size() {
        return write_offset;
    }

    inline size_t get_capacity() {
        return capacity;
    }

    static const size_t PAGE_SIZE = 4096;

private:
    static const size_t CACHE_LINE_SIZE = 64;

    void *start_addr {nullptr};

    size_t write_offset;

    size_t read_offset;

    size_t capacity;
};