
#ifndef _CONCURRENT_QUEUE
#define _CONCURRENT_QUEUE

#include <atomic>
#include <condition_variable>
#include <cstdint>
#include <memory>
#include <mutex>

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

template<typename T>
NODE<T>* concurrent_queue<T>::get_tail()
{
	std::lock_guard<std::mutex> tail_lock(m_tail_mtx);

	return m_tail_p;
}

template<typename T>
std::unique_ptr<NODE<T>> concurrent_queue<T>::locked_pop()
{
	std::unique_ptr<NODE<T>> old_head = std::move(m_head_p);

	m_head_p = std::move(old_head->next_p);
	m_qsize--;

	return old_head;
}

template<typename T>
std::unique_lock<std::mutex> concurrent_queue<T>::wait()
{
	std::unique_lock<std::mutex> head_lock(m_head_mtx);

	m_cv.wait(head_lock, [&]{ return m_head_p.get() != get_tail(); });

	return head_lock;
}

template<typename T>
std::unique_ptr<NODE<T>> concurrent_queue<T>::wait_pop()
{
	std::unique_lock<std::mutex> head_lock(wait());

	return locked_pop();
}

template<typename T>
std::unique_ptr<NODE<T>> concurrent_queue<T>::wait_pop(T& value)
{
	std::unique_lock<std::mutex> head_lock(wait());

	value = std::move(*m_head_p->data);

	return locked_pop();
}

template<typename T>
std::unique_ptr<NODE<T>> concurrent_queue<T>::try_pop()
{
	std::lock_guard<std::mutex> head_lock(m_head_mtx);

	if (m_head_p.get() == get_tail()) {
		return std::unique_ptr<NODE<T>>();
	}

	return locked_pop();
}

template<typename T>
std::unique_ptr<NODE<T>> concurrent_queue<T>::try_pop(T& value)
{
	std::lock_guard<std::mutex> head_lock(m_head_mtx);

	if (m_head_p.get() == get_tail()) {
		return std::unique_ptr<NODE<T>>();
	}

	value = std::move(*m_head_p->data);

	return locked_pop();
}

template<typename T>
void concurrent_queue<T>::push(T new_value)
{
	std::shared_ptr<T> new_data(std::make_shared<T>(std::move(new_value)));
	std::unique_ptr<NODE<T>> new_node(new NODE<T>);
	std::lock_guard<std::mutex> tail_lock(m_tail_mtx);

	m_tail_p->data = new_data;
	NODE<T>* const new_tail = new_node.get();
	m_tail_p->next_p = std::move(new_node);
	m_tail_p = new_tail;
	m_qsize++;

	m_cv.notify_one();
}

template<typename T>
std::shared_ptr<T> concurrent_queue<T>::wait_and_pop()
{
	std::unique_ptr<NODE<T>> const old_head = wait_pop();

	return old_head->data;
}

template<typename T>
void concurrent_queue<T>::wait_and_pop(T& value)
{
	std::unique_ptr<NODE<T>> const old_head = wait_pop(value);
	(void)old_head;
}

template<typename T>
std::shared_ptr<T> concurrent_queue<T>::pop()
{
	std::unique_ptr<NODE<T>> old_head = try_pop();

	return old_head ? old_head->data : std::shared_ptr<T>();
}

template<typename T>
bool concurrent_queue<T>::pop(T& value)
{
	std::unique_ptr<NODE<T>> const old_head = try_pop(value);

	return old_head != nullptr;
}

template<typename T>
bool concurrent_queue<T>::empty()
{
	std::lock_guard<std::mutex> head_lock(m_head_mtx);

	return m_head_p.get() == get_tail();
}

#endif
