#include "Iterator.h"

class WitnessPlan : public Plan
{
	friend class WitnessIterator;
public:
	WitnessPlan (char const * const name, Plan * const input);
	~WitnessPlan ();
	Iterator * init () const override;
private:
	Plan * const _input;
}; // class WitnessPlan

class WitnessIterator : public Iterator
{
public:
	WitnessIterator (WitnessPlan const * const plan);
	~WitnessIterator ();
	bool next (Row & row) override;
	void free (Row & row) override;
	size_t getRowCount() const {return _rows;}
	size_t getParity() const{return _parity;}
	size_t getInversions() const {return _inversions;}
private:
	WitnessPlan const * const _plan;
	Iterator * const _input;
	RowCount _rows;
	size_t _parity;   //store XOR of row values
	size_t _inversions; //Number of inversions
	Row _previousRow; //Last observed row
	bool _hasPrevious; //If previous row is valid

}; // class WitnessIterator
