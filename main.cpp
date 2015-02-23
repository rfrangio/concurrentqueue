#define BOOST_ASIO_SEPARATE_COMPILATION
#include <thread>
#include <iostream>
#include <mutex>
#include <thread>
#include <chrono>
#include <boost/thread/shared_mutex.hpp>
#include "concurrent_queue.h"

concurrent_queue<int32_t> cq;
concurrent_queue<int32_t> oq;

void queue_consumer()
{
	int32_t val = 0;

	while(true) {
		cq.wait_and_pop(val);
		//std::cout << "Got value " << val << " \n" << std::flush;
	}
	std::cout << "Last value pop'd " << val << " \n" << std::flush;
}

void queue_producer()
{
	for(int32_t i = 0; i < 10000000; i++) {
		cq.push(i);
	}
	while(!cq.empty()) { 
		std::this_thread::sleep_for(std::chrono::seconds(1));
	}
}

int main()
{
	const int hardware_threads = std::thread::hardware_concurrency() * 2;
	std::thread	*threads_array = new std::thread[hardware_threads];
	char input;

	std::thread prod1(queue_producer);
	std::thread prod2(queue_producer);
	std::thread prod3(queue_producer);

	for(int i = 0; i < hardware_threads; i++) {
		threads_array[i] = std::move(std::thread(queue_consumer));
	}

	prod1.join();
	prod2.join();
	prod3.join();

	std::cout << "Done.\n";
	std::cin >> input;

	for(int i = 0; i < hardware_threads; i++) {
		threads_array[i].detach();
	}

	std::cout << "shared queue is empty: " << (cq.empty() ? "true" : "false") << " \n";
}
