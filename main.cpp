#define BOOST_ASIO_SEPARATE_COMPILATION
#include <thread>
#include <iostream>
#include <mutex>
#include <thread>
#include <chrono>
#include <boost/thread/shared_mutex.hpp>
#include "concurrent_queue.cpp"


concurrent_queue<uint64_t> cq;
concurrent_queue<char> oq;
const int production_bundle = 10000000;

void queue_consumer()
{
	uint64_t val = 0;

	while(true) {
		cq.wait_and_pop(val);
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
	const int hardware_threads = std::thread::hardware_concurrency() * 2;
	std::thread	*threads_array = new std::thread[hardware_threads];
	char input;

	for(int i = 0; i < hardware_threads; i++) {
		threads_array[i] = std::move(std::thread(queue_consumer));
	}

	std::thread prod1(queue_producer);
	std::thread prod2(queue_producer);
	std::thread prod3(queue_producer);

	while(!cq.empty()) {
		std::cout << "shared queue size " << cq.get_size() << std::endl;
		std::this_thread::sleep_for(std::chrono::seconds(1));
	}
	std::cout << "shared queue size " << cq.get_size() << std::endl;

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
