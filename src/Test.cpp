#include "Iterator.h"
#include "Scan.h"
#include "Filter.h"
#include "Sort.h"
#include "Witness.h"

#include <iostream>
#include <chrono>

class Timer {
	using TimePoint = typename std::chrono::time_point<std::chrono::_V2::system_clock, std::chrono::_V2::system_clock::duration>;
public:
	Timer() {
		start = std::chrono::high_resolution_clock::now();
	}

	~Timer() {
		end = std::chrono::high_resolution_clock::now();
		auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end-start);
		printf("Test took %0.2f ms\n", duration.count());
	}
private:
	TimePoint start, end;
};

#define TIME_TEST Timer timer;

void run_test(uint32_t num_rows) {
	Plan * const plan =
			new WitnessPlan ("output",
				new SortPlan ("*** The main thing! ***",
					new WitnessPlan ("input",
						new FilterPlan ("half",
							new ScanPlan ("source", num_rows)
						)
					)
				)
			);

	Iterator * const it = plan->init ();
	it->run ();
	delete it;

	delete plan;
}

/**
 * Sort a small number of records that can fit within a cache run
 */
void test_cache_run_sort() {
	TIME_TEST
	printf("Running test for sorting a single cache-sized run...\n");
	run_test(10);
}

/**
 * Checks internal merge sort
 */
void test_internal_merge_sort1() {
	TIME_TEST
	printf("Running test for checking internal merge sort (num_rows=1000)...\n");
	run_test(1000);
}

/**
 * Checks internal merge sort
 */
void test_internal_merge_sort2() {
	TIME_TEST
	printf("Running test for checking internal merge sort (num_rows=2000)...\n");
	run_test(2000);
}

/**
 * Checks external merge sort
 */
void test_external_merge_sort1() {
	TIME_TEST
	printf("Running test for checking external merge sort (num_rows=10000)...\n");
	run_test(10000);
}

/**
 * Checks external merge sort
 */
void test_external_merge_sort2() {
	TIME_TEST
	printf("Running test for checking external merge sort (num_rows=100000)...\n");
	run_test(100000);
}


int main (int argc, char * argv [])
{
	TRACE (TRACE_VAL);	
	printf("Starting tests...");

	test_cache_run_sort();
	test_internal_merge_sort1();
	test_internal_merge_sort2();
	test_external_merge_sort1();
	test_external_merge_sort2();

	printf("Done\n");
	return 0;
} // main
