#include "Iterator.h"

Row::Row ()
{
	TRACE (TRACE_VAL);
} // Row::Row

Row::~Row ()
{
	TRACE (TRACE_VAL);
} // Row::~Row

Plan::Plan (char const * const name)
	: _name (name)
{
	TRACE (TRACE_VAL);
} // Plan::Plan

Plan::~Plan ()
{
	TRACE (TRACE_VAL);
} // Plan::~Plan

Iterator::Iterator () : _rows (0)
{
	TRACE (TRACE_VAL);
} // Iterator::Iterator

Iterator::~Iterator ()
{
	TRACE (TRACE_VAL);
} // Iterator::~Iterator

void Iterator::run ()
{
	TRACE (TRACE_VAL);

	for (Row row;  next (row);  free (row))
		++ _rows;

	traceprintf ("entire plan produced %lu rows\n",
			(unsigned long) _rows);
} // Iterator::run
