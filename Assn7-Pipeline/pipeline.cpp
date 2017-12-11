// Note: Uses an atomic to decrement nprod.
#include <atomic>
#include <cassert>
#include <condition_variable>
#include <cstddef>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <mutex>
#include <queue>
#include <thread>
#include "pipeline.h"
using namespace std;

int main() {
    vector<thread> prods, filters, cons;
    for (size_t i = 0; i < Pipeline::ncons; ++i)
        cons.push_back(thread(&Pipeline::consume, i));
	for (size_t i = 0; i < Pipeline::nfilts; ++i)
		filters.push_back(thread(&Pipeline::filter));
    for (size_t i = 0; i < Pipeline::nprods; ++i)
        prods.push_back(thread(&Pipeline::produce,i));

    // Join all threads
    for (auto &p: prods)
        p.join();
	for (auto &f : filters)
		f.join();
    for (auto &c: cons)
        c.join();

	cin.get();
}