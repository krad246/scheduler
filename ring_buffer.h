/*
 * ring_buffer.h
 *
 *  Created on: Sep 20, 2019
 *      Author: krad2
 */

#ifndef RING_BUFFER_H_
#define RING_BUFFER_H_

#include <cstdint>

#include <memory>

template <class T>
class ring_buffer {
public:
	explicit ring_buffer(std::size_t size);

	void put(T item);
	T get();

	void reset();

	bool empty() const;
	bool full() const;

	std::size_t capacity() const;
	std::size_t size() const;

private:
	std::unique_ptr<T[]> buf_;
	std::size_t head_ = 0;
	std::size_t tail_ = 0;
	const std::size_t max_size_;
	bool full_ = 0;
};

#include <ring_buffer.cpp>

#endif /* RING_BUFFER_H_ */
