#include "Iterator.h"
#include "Scan.h"
#include "Filter.h"
#include "Sort.h"
#include "Witness.h"

#include <iostream>
#include <chrono>

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
	auto start = std::chrono::high_resolution_clock::now();
	printf("\n***** Running test for sorting a single cache-sized run *****\n");
	run_test(10);
	auto end = std::chrono::high_resolution_clock::now();
	auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end-start);
	std::cout << "Took: " << duration.count()/1000.0f << " ms\n";
}

/**
 * Checks internal merge sort
 */
void test_internal_merge_sort1() {
	auto start = std::chrono::high_resolution_clock::now();
	printf("\n***** Running test for checking internal merge sort (num_rows=1000) *****\n");
	run_test(1000);
	auto end = std::chrono::high_resolution_clock::now();
	auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end-start);
	std::cout << "Took: " << duration.count()/1000.0 << " ms\n";
}

/**
 * Checks internal merge sort
 */
void test_internal_merge_sort2() {
	auto start = std::chrono::high_resolution_clock::now();
	printf("\n***** Running test for checking internal merge sort (num_rows=2000) *****\n");
	run_test(2000);
	auto end = std::chrono::high_resolution_clock::now();
	auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end-start);
	std::cout << "Took: " << duration.count()/1000.0f << " ms\n";
}

/**
 * Checks external merge sort
 */
void test_external_merge_sort1() {
	auto start = std::chrono::high_resolution_clock::now();
	printf("\n***** Running test for checking external merge sort (num_rows=10000) *****\n");
	run_test(10000);
	auto end = std::chrono::high_resolution_clock::now();
	auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end-start);
	std::cout << "Took: " << duration.count()/1000.0f << " ms\n";
}

/**
 * Checks external merge sort
 */
void test_external_merge_sort2() {
	auto start = std::chrono::high_resolution_clock::now();
	printf("\n***** Running test for checking external merge sort (num_rows=100000) *****\n");
	run_test(100000);
	auto end = std::chrono::high_resolution_clock::now();
	auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end-start);
	std::cout << "Took: " << duration.count()/1000.0f << " ms\n";
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

	printf("\nCompleted tests\n");
	return 0;
} // main
