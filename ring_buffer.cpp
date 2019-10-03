/*
 * ring_buffer.cpp
 *
 *  Created on: Sep 20, 2019
 *      Author: krad2
 */

#ifndef RING_BUFFER_CPP_
#define RING_BUFFER_CPP_

#include <ring_buffer.h>

template <class T>
ring_buffer<T>::ring_buffer(std::size_t size) : buf_(std::unique_ptr<T[]>(new T[size])), max_size_(size) { }

template <class T>
inline void ring_buffer<T>::put(T item) {
	buf_[head_] = item;	// Write data

	if (full_) {	// If full, must drop the oldest element
		tail_ = (tail_ + 1) % max_size_;
	}

	head_ = (head_ + 1) % max_size_;	// Advance head pointer to signify push

	full_ = head_ == tail_;	// Update status
}

template <class T>
inline T ring_buffer<T>::get() {
	if (empty()) {	// If empty, return NULL
		return T();
	}

	// Read data and advance the tail (we now have a free space)
	auto val = buf_[tail_];
	full_ = false;
	tail_ = (tail_ + 1) % max_size_;

	return val;
}

template <class T>
inline void ring_buffer<T>::reset() {
	head_ = tail_;	// Sets the size to zero
	full_ = false;
}

template <class T>
inline bool ring_buffer<T>::empty() const {
	// If head and tail are equal, we are empty
	return (!full_ && (head_ == tail_));
}

template <class T>
inline bool ring_buffer<T>::full() const {
	// If tail is ahead the head by 1, we are full
	return full_;
}

template <class T>
inline std::size_t ring_buffer<T>::capacity() const {
	return max_size_;
}

template <class T>
std::size_t ring_buffer<T>::size() const {
	std::size_t size = max_size_;

	if (!full_) {	// Does some pointer arithmetic depending on the locations of stuff
		if (head_ >= tail_) {
			size = head_ - tail_;
		} else {
			size = max_size_ + head_ - tail_;
		}
	}

	return size;
}

#endif
