#include "Witness.h"
#include <iostream>

WitnessPlan::WitnessPlan (char const * const name, Plan * const input)
	: Plan (name), _input (input)
{
	TRACE (TRACE_VAL);
} // WitnessPlan::WitnessPlan

WitnessPlan::~WitnessPlan ()
{
	TRACE (TRACE_VAL);
	delete _input;
} // WitnessPlan::~WitnessPlan

Iterator * WitnessPlan::init () const
{
	TRACE (TRACE_VAL);
	return new WitnessIterator (this);
} // WitnessPlan::init

WitnessIterator::WitnessIterator (WitnessPlan const * const plan) :
	_plan (plan), _input (plan->_input->init ()),
	_rows (0), first (true), inversions(0)
{
	TRACE (TRACE_VAL);
} // WitnessIterator::WitnessIterator

WitnessIterator::~WitnessIterator ()
{
	TRACE (TRACE_VAL);

	delete _input;

	traceprintf ("%s witnessed %lu rows, %lu inversions\n",
			_plan->_name,
			(unsigned long) (_rows),
			(unsigned long) (inversions));
	std::cout << "Witness partity: " << witness_record.to_string() << "\n";
} // WitnessIterator::~WitnessIterator

bool WitnessIterator::next (Row & row)
{
	TRACE (TRACE_VAL);
	if ( ! _input->next (row))  return false;
	++ _rows;
	if (first) {
		first = false;
	} else if (!previous.naive_lte(row)) {
		++inversions;
	}
	witness_record.witness(row);
	return true;
} // WitnessIterator::next

void WitnessIterator::free (Row & row)
{
	TRACE (TRACE_VAL);
	previous = row;
	_input->free (row);
} // WitnessIterator::free
