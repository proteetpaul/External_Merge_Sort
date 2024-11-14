#include <stdlib.h>

class Row
{
public:
	Row ();
	virtual ~Row ();
	// ...
private:
	// ...
}; // class Row


class DataRecord: public Row {
public:
    DataRecord(uint32_t x, uint32_t y, uint32_t z)
        : Row(), c1(x), c2(y), c3(z) {}

    virtual ~DataRecord() = default;

    static DataRecord* generate_random() {
        uint32_t x = rand();
        uint32_t y = rand();
        uint32_t z = rand();
        DataRecord* d = new DataRecord(x, y, z);
        return d;
    }

private:
    uint32_t c1;
    uint32_t c2;
    uint32_t c3;
    char padding[4]; // for alignment
};