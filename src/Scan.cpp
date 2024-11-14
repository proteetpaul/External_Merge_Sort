#include "Scan.h"
#include "Record.h"
#include <memory>

ScanPlan::ScanPlan (char const * const name, RowCount const count)
	: Plan (name), _count (count)
{
	TRACE (TRACE_VAL);
} // ScanPlan::ScanPlan

ScanPlan::~ScanPlan ()
{
	TRACE (TRACE_VAL);
} // ScanPlan::~ScanPlan

Iterator * ScanPlan::init () const
{
	TRACE (TRACE_VAL);
	return new ScanIterator (this);
} // ScanPlan::init

ScanIterator::ScanIterator (ScanPlan const * const plan) :
	_plan (plan), _count (0)
{
	TRACE (TRACE_VAL);
} // ScanIterator::ScanIterator

ScanIterator::~ScanIterator ()
{
	TRACE (TRACE_VAL);
	traceprintf ("produced %lu of %lu rows\n",
			(unsigned long) (_count),
			(unsigned long) (_plan->_count));
} // ScanIterator::~ScanIterator

bool ScanIterator::next (Row & row)
{
	TRACE (TRACE_VAL);

	if (_count >= _plan->_count)
		return false;

	++ _count;
	return true;
} // ScanIterator::next

void ScanIterator::free (Row & row)
{
	TRACE (TRACE_VAL);
	Row* new_row = dynamic_cast<Row*>(DataRecord::generate_random());
	row = std::move(*new_row);	
} // ScanIterator::free
