#include <algorithm>
#include <cstdint>
#include <thread>
#include <iostream>
#include <chrono>
#include <limits>
#include <vector>
#include "concurrent_queue.h"


concurrent_queue<uint64_t> cq;
const int production_bundle = 10000000;
constexpr uint64_t shutdown_value = std::numeric_limits<uint64_t>::max();

void queue_consumer()
{
	uint64_t val = 0;

	while (true) {
		cq.wait_and_pop(val);
		if (val == shutdown_value) {
			return;
		}
	}
}

void queue_producer()
{
	for(uint64_t i = 0; i < production_bundle; i++) {
		cq.push(i);
	}

	std::cout << "thread id: " << std::this_thread::get_id() << 
					" done pushing " << production_bundle << " objects. \n";
	while(!cq.empty()) { 
		std::this_thread::sleep_for(std::chrono::seconds(1));
	}
}

int main()
{
	const unsigned int hardware_threads =
		std::max(1u, std::thread::hardware_concurrency() * 2);
	std::vector<std::thread> consumers;
	consumers.reserve(hardware_threads);

	for (unsigned int i = 0; i < hardware_threads; i++) {
		consumers.emplace_back(queue_consumer);
	}

	std::thread prod1(queue_producer);
	std::thread prod2(queue_producer);
	std::thread prod3(queue_producer);

	prod1.join();
	prod2.join();
	prod3.join();

	while (!cq.empty()) {
		std::cout << "shared queue size " << cq.get_size() << std::endl;
		std::this_thread::sleep_for(std::chrono::seconds(1));
	}
	std::cout << "Done.\n";

	for (unsigned int i = 0; i < hardware_threads; i++) {
		cq.push(shutdown_value);
	}

	for (std::thread& consumer : consumers) {
		consumer.join();
	}

	std::cout << "shared queue is empty: " << (cq.empty() ? "true" : "false") << " \n";
}
