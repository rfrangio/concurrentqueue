
#ifndef _CONCURRENT_QUEUE
#define _CONCURRENT_QUEUE 

#define BOOST_ASIO_SEPARATE_COMPILATION
#include <thread>
#include <iostream>
#include <mutex>
#include <thread>
#include <chrono>
#include <boost/thread/shared_mutex.hpp>
#include <memory>

template<typename T>
struct NODE 
{	
	std::shared_ptr<T> data;
	std::unique_ptr<NODE<T>> next_p;
};

template<typename T>
class concurrent_queue
{

private:
	std::mutex m_head_mtx;
	std::unique_ptr<NODE<T>> m_head_p;
	std::mutex	m_tail_mtx;
	NODE<T> *m_tail_p;
	std::condition_variable m_cv;
	mutable boost::shared_mutex m_shared_mtx;
	std::atomic<uint64_t> m_qsize{0};

	NODE<T>* get_tail();
	std::unique_ptr<NODE<T>> locked_pop(); // must have head mtx
	std::unique_lock<std::mutex> wait();
	std::unique_ptr<NODE<T>> wait_pop();
	std::unique_ptr<NODE<T>> wait_pop(T& value);
	std::unique_ptr<NODE<T>> try_pop();
	std::unique_ptr<NODE<T>> try_pop(T& value);

public:
	concurrent_queue() :
		m_head_p(new NODE<T>), m_tail_p(m_head_p.get())
	{}

	concurrent_queue(const concurrent_queue& other) = delete;
	concurrent_queue& operator=(const concurrent_queue& other) = delete;

	uint64_t get_size() { return m_qsize; }
	void push(T new_value);
	std::shared_ptr<T> wait_and_pop();
	void wait_and_pop(T& value);
	std::shared_ptr<T> pop();
	bool pop(T& value);
	bool empty();
};

#endif

