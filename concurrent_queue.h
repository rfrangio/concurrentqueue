
#ifndef _CONCURRENT_QUEUE
#define _CONCURRENT_QUEUE 

#define BOOST_ASIO_SEPARATE_COMPILATION
#include <thread>
#include <iostream>
#include <mutex>
#include <thread>
#include <chrono>
#include <boost/thread/shared_mutex.hpp>

template<typename T>
class concurrent_queue
{
private:
	struct node
	{	
		std::shared_ptr<T> data;
		std::unique_ptr<node> next;
	};

	std::mutex m_head_mtx;
	std::unique_ptr<node> m_head_p;
	std::mutex	m_tail_mtx;
	node *m_tail_p;
	std::condition_variable m_cv;
    mutable boost::shared_mutex m_shared_mtx;

	node* get_tail();
	std::unique_ptr<node> locked_pop(); // must have head mtx
	std::unique_lock<std::mutex> wait();
	std::unique_ptr<node> wait_pop();
	std::unique_ptr<node> wait_pop(T& value);
	std::unique_ptr<node> try_pop();
	std::unique_ptr<node> try_pop(T& value);

public:
	concurrent_queue() :
		m_head_p(new node), m_tail_p(m_head_p.get())
	{}

	concurrent_queue(const concurrent_queue& other) = delete;
	concurrent_queue& operator=(const concurrent_queue& other) = delete;

	void push(T new_value);
	std::shared_ptr<T> wait_and_pop();
	void wait_and_pop(T& value);
	std::shared_ptr<T> pop();
	bool pop(T& value);
	bool empty();
};

#endif

