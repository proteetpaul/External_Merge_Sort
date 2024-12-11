#include "Filter.h"
#include <random>

FilterPlan::FilterPlan (char const * const name, Plan * const input)
	: Plan (name), _input (input)
{
	TRACE (TRACE_VAL);
} // FilterPlan::FilterPlan

FilterPlan::~FilterPlan ()
{
	TRACE (TRACE_VAL);
	delete _input;
} // FilterPlan::~FilterPlan

Iterator * FilterPlan::init () const
{
	TRACE (TRACE_VAL);
	return new FilterIterator (this);
} // FilterPlan::init

FilterIterator::FilterIterator (FilterPlan const * const plan) :
	_plan (plan), _input (plan->_input->init ()),
	_consumed (0), _produced (0)
{
	TRACE (TRACE_VAL);
} // FilterIterator::FilterIterator

FilterIterator::~FilterIterator ()
{
	TRACE (TRACE_VAL);

	delete _input;

	traceprintf ("produced %lu of %lu rows\n",
			(unsigned long) (_produced),
			(unsigned long) (_consumed));
} // FilterIterator::~FilterIterator

bool FilterIterator::next (Row & row)
{
	TRACE (TRACE_VAL);

	for (;;)
	{
		if ( ! _input->next (row))  return false;
		++ _consumed;
		if (_consumed % 2)
			break;

		_input->free (row);
	}

	++ _produced;
	return true;
} // FilterIterator::next

void FilterIterator::free (Row & row)
{
	TRACE (TRACE_VAL);
	_input->free (row);
} // FilterIterator::free
