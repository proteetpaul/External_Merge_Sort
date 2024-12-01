#include <stdlib.h>
// OVC is represented by a 64 bit integer. First 32 bits -> offset, next 32 bits -> value
#define OVC uint64_t

class Row
{
public:
	Row ();
	virtual ~Row ();
	// ...
private:
	// ...
}; // class Row


// Arity for ascending OVC
const uint64_t ARITY = 3;

class DataRecord: public Row {
public:
    DataRecord(uint32_t x, uint32_t y, uint32_t z)
        : Row() {
        values[0] = x;
        values[1] = y;
        values[2] = z;
        ovc = ARITY * (1ll<<31) + x;
    }

    virtual ~DataRecord() = default;

    static DataRecord* generate_random() {
        uint32_t x = rand();
        uint32_t y = rand();
        uint32_t z = rand();
        DataRecord* d = new DataRecord(x, y, z);
        return d;
    }

    // Returns a record representing an infinite value (used as an invalid sentinel value in tournament tree)
    static DataRecord inf() {
        DataRecord d {INT32_MAX, INT32_MAX, INT32_MAX};
        return d;
    }

    inline bool operator <(DataRecord &other) {
        if (ovc != other.ovc) {
            return ovc < other.ovc;
        }
        uint32_t pos = ARITY - (ovc >> 32);
        bool res;
        uint32_t i=pos+1;
        for (; i<ARITY; i++) {
            if (values[i] < other.values[i]) {
                other.ovc = (ARITY-i) * (1ll<<32) + other.values[i];
                return true;
            } else if (values[i] > other.values[i]) {
                ovc = (ARITY-i) * (1ll<<32) + values[i];
                return false;
            }
        }
        return false;
    }

    inline bool operator ==(DataRecord &other) {
        return values[0] == other.values[1] && values[1] == other.values[1] && values[2] == other.values[2];
    }

    OVC ovc;

private:
    uint32_t values[3];

    char padding[12]; // for alignment. Since the size of a single node is 32 bytes, a single node will not cross cache line boundaries
};