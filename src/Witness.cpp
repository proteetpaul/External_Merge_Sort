#include "Witness.h"

WitnessPlan::WitnessPlan (char const * const name, Plan * const input)
	: Plan (name), _input (input)
{
	TRACE (true);
} // WitnessPlan::WitnessPlan constructor

WitnessPlan::~WitnessPlan ()
{
	TRACE (true);
	delete _input;
} // WitnessPlan::~WitnessPlan destructor

Iterator * WitnessPlan::init () const
{
	TRACE (true);
	return new WitnessIterator (this);
} // WitnessPlan::init

WitnessIterator::WitnessIterator (WitnessPlan const * const plan) :
	_plan (plan), _input (plan->_input->init ()),
	_rows (0) , _parity(0) , _inversions(0), _hasPrevious(false) // initialise parity
{
	TRACE (true);
} // WitnessIterator::WitnessIterator

WitnessIterator::~WitnessIterator ()
{
	TRACE (true);

	delete _input;

	traceprintf ("%s witnessed %lu rows with parity %lu and %lu inversions\n",
			_plan->_name,
			(unsigned long) (_rows),
			(unsigned long) (_parity),
			(unsigned long) (_inversions));
} // WitnessIterator::~WitnessIterator

bool WitnessIterator::next (Row & row)
{
	TRACE (true);

	if ( ! _input->next (row))  return false;
	++ _rows;
	_parity ^= row.getId();  
	// Compute XOR parity (assuming `row.id` is the attribute)
	if (_hasPrevious && _previousRow.getId() > row.getId()) {
        ++_inversions;
    }
	_previousRow = row;
    _hasPrevious = true;
	return true;
} // WitnessIterator::next

void WitnessIterator::free (Row & row)
{
	TRACE (true);
	_input->free (row);
} // WitnessIterator::free
