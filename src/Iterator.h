#pragma once

#include "defs.h"
#include "Record.h"
#include <vector>
#include <boost/align/aligned_allocator.hpp>

typedef uint64_t RowCount;

class Plan
{
	friend class Iterator;
public:
	Plan (char const * const name);
	virtual ~Plan ();
	virtual class Iterator * init () const = 0;
protected:
	char const * const _name;
private:
}; // class Plan

class Iterator
{
public:
	Iterator ();
	virtual ~Iterator ();
	void run ();
	virtual bool next (Row & row) = 0;
	virtual void free (Row & row) = 0;
private:
	RowCount _rows;

protected:
	std::vector<Row, boost::alignment::aligned_allocator<Row, 64>> records;
}; // class Iterator
