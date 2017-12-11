#ifndef PIPELINE
#define PIPELINE
#include <atomic>
#include <cassert>
#include <cstddef>
#include <cstdlib>
#include <condition_variable>
#include <fstream>
#include <iostream>
#include <mutex>
#include <string>
#include <queue>

class Pipeline {
	static std::queue<int> q, q2;
	static std::condition_variable q_cond, q2_cond;
	static std::mutex q_sync, q2_sync, print;
	static std::atomic_size_t nprod, nfilt, num;
public:
	static const size_t nprods = 6, nfilts = 5, ncons = 11, NUMS_PER_PROD = 500, NUMS_PER_FILT = 600;

	static void consume(int i) {
		std::string filename = "Group" + std::to_string(i) + ".txt";
		std::ofstream out(filename);
		std::vector<size_t> count(ncons);
		for (;;) {
			// Get lock for sync mutex
			std::unique_lock<std::mutex> q2lck(q2_sync);

			// Wait for queue to have something to process
			q2_cond.wait(q2lck, []() {return !q2.empty() || !nfilt; });
			if (q2.empty()) {
				assert(!nfilt);
				break;
			}
			auto x = q2.front();
			if (x % 11 == i) {
				q2.pop();
				count[i]++;
				out << x << " consumed\n";	// Print trace of consumption
			}
			q2lck.unlock();
		}
		out << std::flush;
		std::lock_guard<std::mutex> plck(print);
		std::cout << "Group" << i << " has " << count[i] << " numbers." << std::endl;
	}

	static void filter() {
		for (;;) {
			// Get lock for sync mutex
			std::unique_lock<std::mutex> qlck(q_sync);

			// Wait for queue to have something to process
			q_cond.wait(qlck, []() {return !q.empty() || !nprod; });
			if (q.empty()) {
				assert(!nprod);
				break;
			}
			auto x = q.front();
			q.pop();
			num++;
			qlck.unlock();

			//Push x on q2 if appropriate
			if (num % 5 != 0 && num % 13 != 0) {
				std::unique_lock<std::mutex> q2lck(q2_sync);
				q2.push(x);
				q2lck.unlock();
				q2_cond.notify_one();
			}
		}
		// Notify consumers that a filter has shut down
		--nfilt;
		q2_cond.notify_all();
	}

	static void produce(int i) {
		// Generate 3000 random ints
		srand(time(nullptr) + i*(i + 1));
		for (int i = 0; i < NUMS_PER_PROD; ++i) {
			int n = rand();     // Get random int

			// Get lock for queue; push int
			std::unique_lock<std::mutex> qlck(q_sync);
			q.push(n);
			qlck.unlock();
			q_cond.notify_one();
		}
		// Notify filters that a producer has shut down
		--nprod;
		q_cond.notify_all();
	}
};

std::queue<int> Pipeline::q, Pipeline::q2;
std::condition_variable Pipeline::q_cond, Pipeline::q2_cond;
std::mutex Pipeline::q_sync, Pipeline::q2_sync, Pipeline::print;
std::atomic_size_t Pipeline::nprod(nprods), Pipeline::nfilt(nfilts), Pipeline::num(0);
#endif // !PIPELINE

