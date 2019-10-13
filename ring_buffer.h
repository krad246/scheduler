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

// Ring buffer class for kernel data structures

template <class T>
class ring_buffer {
public:
	ring_buffer();
	ring_buffer(std::size_t size);

	ring_buffer(const ring_buffer &other);
	ring_buffer &operator=(const ring_buffer &other);

	inline void put(T item);	// Push
	inline T get();				// Pop

	inline void reset();		// Clear

	inline bool empty() const;
	inline bool full() const;

	inline std::size_t capacity() const;
	inline std::size_t size() const;

private:
	std::unique_ptr<T[]> buf_;
	std::size_t head_ = 0;
	std::size_t tail_ = 0;
	const std::size_t max_size_;
	bool full_ = 0;
};

#include <ring_buffer.cpp>

#endif /* RING_BUFFER_H_ */
