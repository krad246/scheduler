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
void ring_buffer<T>::put(T item) {

		buf_[head_] = item;

		if(full_)
		{
			tail_ = (tail_ + 1) % max_size_;
		}

		head_ = (head_ + 1) % max_size_;

		full_ = head_ == tail_;
}

template <class T>
T ring_buffer<T>::get() {

	if(empty())
	{
		return T();
	}

	//Read data and advance the tail (we now have a free space)
	auto val = buf_[tail_];
	full_ = false;
	tail_ = (tail_ + 1) % max_size_;

	return val;
}

template <class T>
void ring_buffer<T>::reset() {
		head_ = tail_;
		full_ = false;
}

template <class T>
bool ring_buffer<T>::empty() const {
		//if head and tail are equal, we are empty
		return (!full_ && (head_ == tail_));
}

template <class T>
bool ring_buffer<T>::full() const {
		//If tail is ahead the head by 1, we are full
		return full_;
}

template <class T>
	std::size_t ring_buffer<T>::capacity() const
	{
		return max_size_;
	}
template <class T>
	std::size_t ring_buffer<T>::size() const
	{
		std::size_t size = max_size_;

		if(!full_)
		{
			if(head_ >= tail_)
			{
				size = head_ - tail_;
			}
			else
			{
				size = max_size_ + head_ - tail_;
			}
		}

		return size;
	}

#endif
