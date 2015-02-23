#include "concurrent_queue.h"

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

	m_cv.wait(head_lock, [&]{return m_head_p.get() != get_tail();});

	return std::move(head_lock);
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

	if(m_head_p.get() == get_tail()) {
		return std::unique_ptr<NODE<T>>();
	}

	return locked_pop();
}

template<typename T>
std::unique_ptr<NODE<T>> concurrent_queue<T>::try_pop(T& value)
{
	std::lock_guard<std::mutex> head_lock(m_head_mtx);

	if(m_head_p.get() == get_tail()) {
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

	m_tail_p->data=new_data;
	NODE<T>* const new_tail=new_node.get();
	m_tail_p->next_p=std::move(new_node);
	m_tail_p=new_tail;
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

	return old_head == nullptr ? false : true;
}

template<typename T>
bool concurrent_queue<T>::empty()
{
	std::lock_guard<std::mutex> head_lock(m_head_mtx);

	return (m_head_p.get() == get_tail());
}

//template class concurrent_queue<int32_t>;

