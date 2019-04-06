/*
 * List.h
 *
 *  Created on: Apr 5, 2019
 *      Author: krad2
 */

#ifndef LIST_H_
#define LIST_H_

#include <cstddef>

template <class T>
class List;

template <class T>
class ListIterator;

template <class T>
class ListNode {
private:
	friend class List<T>;
	friend class ListIterator<T>;

	ListNode();
	ListNode(T& data);
	~ListNode();

	T& get(void);

	T data;
	ListNode<T> *next;
	ListNode<T> *prev;
};

template <class T>
class ListIterator {
public:
	ListIterator<T>& operator++();
	ListIterator<T> operator++(int);
	ListIterator<T>& operator--();
	ListIterator<T> operator--(int);

	T& operator*();

private:
	friend class List<T>;

	ListIterator();
	ListIterator(ListNode<T> *ref);

	ListNode<T> *iter;
	std::size_t idx = 0;
};

template <class T>
class List {
public:
	List();
	~List();

	ListIterator<T> begin(void);
	ListIterator<T> end(void);

	void push_back(T& data);
	void push_front(T& data);

	T& pop_back(void);
	T& pop_front(void);

	T& pop(std::size_t idx);
	T& pop(ListIterator<T> &location);

	void clear(void);

	T& front(void);
	T& back(void);

	T& operator[](std::size_t idx);
	ListIterator<T> find(T& data);

	std::size_t size(void);

private:
	friend class ListIterator<T>;

	ListNode<T> *head;
	ListNode<T> *tail;

	std::size_t count;
};

template <class T>
ListNode<T>::ListNode() : data(T()), next(nullptr), prev(nullptr) { }

template <class T>
ListNode<T>::ListNode(T& data) : data(data), next(nullptr), prev(nullptr) { }

template <class T>
ListNode<T>::~ListNode() { }

template <class T>
inline T& ListNode<T>::get(void) {
	return data;
}

template <class T>
List<T>::List() : head(nullptr), tail(nullptr), count(0) { }

template <class T>
List<T>::~List() {
	clear();
}

template <class T>
ListIterator<T>::ListIterator() : iter(nullptr) { }

template <class T>
ListIterator<T>::ListIterator(ListNode<T> *ref) : iter(ref) { }

template <class T>
inline ListIterator<T>& ListIterator<T>::operator++() {
	iter = iter->next;
	++idx;
	return *this;
}

template <class T>
inline ListIterator<T> ListIterator<T>::operator++(int) {
	ListIterator<T> tmp = *this;
	operator++();
	return tmp;
}

template <class T>
inline ListIterator<T>& ListIterator<T>::operator--() {
	iter = iter->prev;
	--idx;
	return *this;
}

template <class T>
inline ListIterator<T> ListIterator<T>::operator--(int) {
	ListIterator<T> tmp = *this;
	operator--();
	return tmp;
}

template <class T>
inline ListIterator<T>& operator+=(ListIterator<T>& lhs, std::size_t rhs) {
   for (std::size_t i = 0; i < rhs; i++) {
	   ++lhs;
   }

   return lhs;
}

template <class T>
inline ListIterator<T> operator+(ListIterator<T> lhs, std::size_t rhs) {
   return lhs += rhs;
}

template <class T>
inline ListIterator<T>& operator-=(ListIterator<T>& lhs, std::size_t rhs) {
   for (std::size_t i = 0; i < rhs; i++) {
	   --lhs;
   }

   return lhs;
}

template <class T>
inline ListIterator<T> operator-(ListIterator<T> lhs, std::size_t rhs) {
   return lhs -= rhs;
}

template <class T>
inline T& ListIterator<T>::operator*() {
	return iter->get();
}

template <class T>
ListIterator<T> List<T>::begin(void) {
	volatile ListNode<T> *x = head;
	ListIterator<T> iter =  ListIterator<T>(head);
	return iter;
}

template <class T>
inline ListIterator<T> List<T>::end(void) {
	ListIterator<T> iter =  ListIterator<T>(tail);
	iter.idx = count - 1;
	return iter;
}

template <class T>
void List<T>::push_back(T& data) {
	ListNode<T> *end = new ListNode<T>(data);

	if (head == nullptr) {
		head = end;
		tail = end;
	} else {
		tail->next = end;
		end->prev = tail;

		tail = tail->next;
	}

	count++;
}

template <class T>
void List<T>::push_front(T& data) {
	ListNode<T> *end = new ListNode<T>(data);

	if (head == nullptr) {
		head = end;
		tail = end;
	} else {
		end->next = head;
		head->prev = end;

		head = head->prev;
	}

	count++;
}

template <class T>
T& List<T>::pop_back(void) {
    ListNode<T> ret;
	if (tail->prev) {
	    tail = tail->prev;

	    ret = *tail->next;
	    delete tail->next;

	    tail->next = nullptr;
	} else {
	    ret = *tail;

	    delete tail;
	    tail = nullptr;
	}

	count--;

	return ret.get();
}

template <class T>
T& List<T>::pop_front(void) {
	ListNode<T> ret;
	if (head->next) {
	    head = head->next;

	    ret = *head->prev;
	    delete head->prev;

	    head->prev = nullptr;
	} else {
	    ret = *head;

	    delete head;
	    head = nullptr;
	}

	count--;

	return ret.get();
}

template <class T>
T& List<T>::pop(std::size_t idx) {
	T ret;

	ListIterator<T> listIt = begin();
	for (std::size_t i = 0; i < idx; ++i) {
		++iter;
	}

	ListNode<T> *left = listIt.iter->prev;
	ListNode<T> *right = listIt.iter->next;

	left->next = right;
	right->prev = left;

	ret = *listIt;
	delete listIt.iter;

	return ret;
}

template <class T>
T& List<T>::pop(ListIterator<T> &location) {
	T ret;

	ListIterator<T> listIt = begin();
	ListNode<T> *left = listIt.iter->prev;
	ListNode<T> *right = listIt.iter->next;

	left->next = right;
	right->prev = left;

	ret = *listIt;
	delete listIt.iter;

	return ret;
}

template <class T>
void List<T>::clear(void) {
	while (count > 0) pop_back();
}

template <class T>
inline T& List<T>::front(void) {
	return head->get();
}

template <class T>
inline T& List<T>::back(void) {
	return tail->get();
}

template <class T>
T& List<T>::operator[](std::size_t idx) {
	ListIterator<T> iter = begin();
	for (std::size_t i = 0; i < count; ++i) {
		++iter;
	}

	return *iter;
}

template <class T>
ListIterator<T> List<T>::find(T& data) {
	ListIterator<T> iter = begin();
	for (std::size_t i = 0; i < count; ++i) {
		++iter;
	}

	return iter;
}

template <class T>
inline std::size_t List<T>::size(void) {
	return count;
}

#endif /* LIST_H_ */
