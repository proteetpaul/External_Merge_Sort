#pragma once

#include <stdlib.h>
#include <cstdint>
#include <string>
#include <iostream>
// OVC is represented by a 64 bit integer. First 32 bits -> offset, next 32 bits -> value
#define OVC uint64_t

// class Row
// {
// public:
// 	Row ();
// 	virtual ~Row ();
// 	// ...
// private:
// 	// ...
// }; // class Row


// Arity for ascending OVC
const uint64_t ARITY = 3;
const uint64_t OFFSET_MULTIPLIER = (1ll<<32);

class Row {
public:
    Row(uint32_t x, uint32_t y, uint32_t z) {
        values[0] = x;
        values[1] = y;
        values[2] = z;
        ovc = ARITY * OFFSET_MULTIPLIER + x;
    }

    Row() {
        values[0] = 0;
        values[1] = 0;
        values[2] = 0;
        ovc = ARITY * OFFSET_MULTIPLIER;
    }

    virtual ~Row() = default;

    static Row* generate_random() {
        uint32_t x = rand();
        uint32_t y = rand();
        uint32_t z = rand();
        Row* d = new Row(x, y, z);
        return d;
    }

    // Returns a record representing an infinite value (used as an invalid sentinel value in tournament tree)
    static Row inf() {
        Row d {UINT32_MAX, UINT32_MAX, UINT32_MAX};
        // std::cout << "Inf row OVC: " << d.ovc << "\n";
        // std::cout << "RANDMAX: " << RAND_MAX << "\n";
        return d;
    }

    inline bool operator <(Row &other) {
        if (ovc != other.ovc) {
            return ovc < other.ovc;
        }
        uint32_t pos = ARITY - (ovc >> 32);
        uint32_t i=pos+1;
        for (; i<ARITY; i++) {
            if (values[i] < other.values[i]) {
                other.ovc = (ARITY-i) * OFFSET_MULTIPLIER + other.values[i];
                return true;
            } else if (values[i] > other.values[i]) {
                ovc = (ARITY-i) * OFFSET_MULTIPLIER + values[i];
                return false;
            }
        }
        return false;
    }

    inline bool operator ==(Row &other) {
        return values[0] == other.values[1] && values[1] == other.values[1] && values[2] == other.values[2];
    }

    inline void witness(Row &other) {
        this->values[0] ^= other.values[0];
        this->values[1] ^= other.values[1];
        this->values[2] ^= other.values[2];
    }

    std::string to_string() {
        std::string res = "v0: " + std::to_string(values[0]) + ", v1: " + std::to_string(values[1]) 
            + ", v2: " + std::to_string(values[2]);
        return res;
    }

    OVC ovc;

private:
    uint32_t values[3];

    char padding[4]; // for alignment. Since the size of a single node is 32 bytes, a single node will not cross cache line boundaries
};