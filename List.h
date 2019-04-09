/*
 * List.h
 *
 *  Created on: Apr 5, 2019
 *      Author: krad2
 */

#ifndef LIST_H_
#define LIST_H_

#include <cstddef>

/**
 * Forward declarations for the various list-related classes in this file.
 */

template <class T>
class List;

template <class T>
class ListIterator;

/**
 * Templated linked list node class. Cannot be directly instantiated but works
 * with other nodes to form a doubly-linked circular linked list.
 */

template <class T>
class ListNode {
private:

	/**
	 * All of the other list-related classes need to be able to access this one.
	 */

	friend class List<T>;
	friend class ListIterator<T>;

	/**
	 * Constructors and destructors for nodes.
	 */

	ListNode();
	ListNode(T& data);
	~ListNode();

	/**
	 * Retrieves the contents of a list node.
	 */

	T& get(void);

	/**
	 * Content + linkage elements.
	 */

	T data;
	ListNode<T> *next;
	ListNode<T> *prev;
};

/**
 * Default initializes links to NULL.
 */

template <class T>
ListNode<T>::ListNode() : data(T()), next(nullptr), prev(nullptr) { }

/**
 * Sets data held in the node.
 */

template <class T>
ListNode<T>::ListNode(T& data) : data(data), next(nullptr), prev(nullptr) { }

/**
 * Nodes are killed by the parent class and don't do anything special in the destructor.
 */

template <class T>
ListNode<T>::~ListNode() { }

/**
 * Retrieves underlying data.
 */

template <class T>
inline T& ListNode<T>::get(void) {
	return data;
}

/**
 * List iterator class. Behaves like a C++ vector iterator.
 */

template <class T>
class ListIterator {
public:

	/**
	 * Functions to advance / decrement a ListIterator<T> in various ways.
	 */

	ListIterator<T>& operator++();
	ListIterator<T> operator++(int);
	ListIterator<T>& operator--();
	ListIterator<T> operator--(int);

	/**
	 * Dereference operators returns the underlying data.
	 */

	T& operator*();
	T *operator->();

	/**
	 * Equality comparators.
	 */

	bool operator==(const ListIterator<T>& rhs);
	bool operator!=(const ListIterator<T>& rhs);

	/**
	 * Overload of operator= so that we can assign multiple iterators to the same place.
	 */

	ListIterator<T>& operator=(const ListIterator<T>& rhs);

private:

	/**
	 * Needs to work with a List<T>.
	 */

	friend class List<T>;

	/**
	 * Constructors that either initialize the underlying pointer to an object to NULL or
	 * the passed-in argument.
	 */

	ListIterator();
	ListIterator(ListNode<T> *ref);

	/**
	 * The ListIterator<T> is just basically a glorified pointer to an object.
	 */

	ListNode<T> *node;
};

/**
 * Default behavior is to have no pointer to the node.
 */

template <class T>
ListIterator<T>::ListIterator() : node(nullptr) { }

/**
 * Constructing a ListIterator<T> with a node pointer will set it.
 */

template <class T>
ListIterator<T>::ListIterator(ListNode<T> *ref) : node(ref) { }

/**
 * Defines the ++x operator.
 */

template <class T>
inline ListIterator<T>& ListIterator<T>::operator++() {
	node = node->next;
	return *this;
}

/**
 * Defines the x++ operator. Note that as usual, the old value is returned but the
 * increment happens.
 */

template <class T>
inline ListIterator<T> ListIterator<T>::operator++(int) {
	ListIterator<T> tmp = *this;
	operator++();
	return tmp;
}

/**
 * Defines --x operator.
 */

template <class T>
inline ListIterator<T>& ListIterator<T>::operator--() {
	node = node->prev;
	return *this;
}

/**
 * Same deal as above.
 */

template <class T>
inline ListIterator<T> ListIterator<T>::operator--(int) {
	ListIterator<T> tmp = *this;
	operator--();
	return tmp;
}

/**
 * Retrieves the contents of the internal node and returns that.
 */

template <class T>
inline T& ListIterator<T>::operator*() {
	return node->get();
}

/**
 * Retrieves the contents of the internal node and returns that.
 */

template <class T>
inline T* ListIterator<T>::operator->() {
	return &node->get();
}

/**
 * Assignment operator copies the underlying pointer to the node.
 */

template <class T>
inline ListIterator<T>& ListIterator<T>::operator=(const ListIterator<T>& rhs) {
	if (this == &rhs) return *this;
	node = rhs.node;
	return *this;
}

/**
 * Equality comparators.
 */

template <class T>
inline bool ListIterator<T>::operator==(const ListIterator<T>& rhs) {
	return node == rhs.node;
}

template <class T>
inline bool ListIterator<T>::operator!=(const ListIterator<T>& rhs) {
	return node != rhs.node;
}


/**
 * Class defining a doubly-linked circular list.
 */

template <class T>
class List {
public:

	/**
	 * Default constructor starts with null head and tail pointers.
	 */

	List();

	/**
	 * Destructor deletes all heap-allocated nodes.
	 */

	~List();

	/**
	 * Returns a pointer / iterator to the head node.
	 */

	ListIterator<T> begin(void);

	/**
	 * Returns a pointer / iterator to the tail node.
	 */

	ListIterator<T> end(void);

	/**
	 * Pushes to either front or back of list.
	 */

	void push_back(T& data);
	void push_front(T& data);

	/**
	 * Pops from either back or front of list.
	 */

	void pop_back(void);
	void pop_front(void);

	/**
	 * Pops at an index.
	 */

	void pop(std::size_t idx);
	void pop(ListIterator<T> &NodeToRemove);

	/**
	 * Deletes all elements.
	 */

	void clear(void);

	/**
	 * Access front / back element.
	 */

	T& front(void);
	T& back(void);

	/**
	 * Indexes into the array.
	 */

	T& operator[](std::size_t idx);

	/**
	 * Returns a pointer to an element if it can be found.
	 */

	ListIterator<T> find(T& data);

	/**
	 * Returns size of list.
	 */

	std::size_t size(void);

private:

	friend class ListIterator<T>;

	/**
	 * Maintain a head and tail pointer to build the list.
	 */

	ListNode<T> *head;
	ListNode<T> *tail;

	/**
	 * Maintain a count of the number of elements.
	 */

	std::size_t count;
};

/**
 * Default-initialize everything to NULL.
 */

template <class T>
List<T>::List() : head(nullptr), tail(nullptr), count(0) { }

/**
 * Run clear() to deallocate all of the heap memory associated with the list and then destruct.
 */

template <class T>
List<T>::~List() {
	clear();
}

/**
 * Form an iterator from the head pointer and return it.
 */

template <class T>
inline ListIterator<T> List<T>::begin(void) {
	ListIterator<T> HeadIterator =  ListIterator<T>(head);
	return HeadIterator;
}

/**
 * Form an iterator from the tail pointer and return it.
 */

template <class T>
inline ListIterator<T> List<T>::end(void) {
	ListIterator<T> TailIterator =  ListIterator<T>(tail);
	return TailIterator;
}

/**
 * Pushes to the end / tail of the list.
 */

template <class T>
void List<T>::push_back(T& data) {

	/**
	 * Allocate a new node to store the data.
	 */

	ListNode<T> *end = new ListNode<T>(data);

	/**
	 * If the list is of size 0, then assign the head and tail to the new node.
	 * The links of this node's resulting linked-list are self-referential because
	 * that node is the start and the end of the list.
	 */

	if (head == nullptr) {
		head = end;
		tail = end;
		end->next = end;
		end->prev = end;
	} else {

		/**
		 * Otherwise this node has to fit between the old tail and the head. Assign the 'next'
		 * of the old tail to this new node, and vice versa for the end node.
		 */

		tail->next = end;
		end->prev = tail;

		/**
		 *  Then reassign the 'next' of this new tail node to the head node and vice versa to maintain
		 *  the circular linkage.
		 */

		end->next = head;
		head->prev = end;

		/**
		 * Finally move the tail forward to the new tail.
		 */

		tail = tail->next;
	}

	/**
	 * Increment the count and leave.
	 */

	count++;
}

/**
 * Pushes to head of list.
 */

template <class T>
void List<T>::push_front(T& data) {

	/**
	 * Allocate a new node to store the data.
	 */

	ListNode<T> *end = new ListNode<T>(data);

	/**
	 * If the list is of size 0, then assign the head and tail to the new node.
	 * The links of this node's resulting linked-list are self-referential because
	 * that node is the start and the end of the list.
	 */

	if (head == nullptr) {
		head = end;
		tail = end;
		end->next = end;
		end->prev = end;
	} else {
		/**
		 * Otherwise this node has to fit between the old head and the tail. Assign the 'prev'
		 * of the old head to this new node, and vice versa for the end node.
		 */

		head->prev = end;
		end->next = head;

		/**
		 * This is the new head so its 'prev' is actually the tail. Connect the links from the
		 * new end node to the tail.
		 */

		end->prev = tail;
		tail->next = end;

		/**
		 * Move the back to adjust for the change.
		 */

		head = head->prev;
	}

	/**
	 * Increment the count and leave.
	 */

	count++;
}

/**
 * Deletes at an index given an iterator to that index.
 */

template <class T>
void List<T>::pop(ListIterator<T> &NodeToRemove) {

	/**
	 * Get the links of the node to remove.
	 */

	ListNode<T> *left = NodeToRemove.node->prev;
	ListNode<T> *right = NodeToRemove.node->next;

	/**
	 * Connect these two adjacent nodes together.
	 */

	left->next = right;
	right->prev = left;

	/**
	 * The list has to have some notion of a head and tail so in order
	 * to maintain the ordering we move the head and tail back and forth
	 * appropriately.
	 */

	if (NodeToRemove.node == head) head = head->next;
	if (NodeToRemove.node == tail) tail = tail->prev;

	/**
	 * Delete the node to be removed and decrement the count. Leave with the
	 * content of the node deleted.
	 */

	delete NodeToRemove.node;
	count--;
}

/**
 * Removes a node at an index.
 */

template <class T>
void List<T>::pop(std::size_t idx) {

	/**
	 * Create an iterator, move it to the right place to delete, then
	 * delete the node there.
	 */

	ListIterator<T> HeadIterator = begin();
	for (std::size_t i = 0; i < idx; ++i) {
		++HeadIterator;
	}

	pop(HeadIterator);
}

/**
 * Pops at the head. Same deal.
 */

template <class T>
void List<T>::pop_front(void) {
	ListIterator<T> HeadIterator = ListIterator<T>(head);
	pop(HeadIterator);
}

/**
 * Pops at the tail. Reuses the functions above.
 */

template <class T>
void List<T>::pop_back(void) {
	ListIterator<T> TailIterator = ListIterator<T>(tail);
	pop(TailIterator);
}

/**
 * Repeatedly removes elements from the back until empty.
 */

template <class T>
void List<T>::clear(void) {
	while (count > 0) pop_back();
}

/**
 * Returns the contents of the head node.
 */

template <class T>
inline T& List<T>::front(void) {
	return head->get();
}

/**
 * Returns the contents of the tail node.
 */

template <class T>
inline T& List<T>::back(void) {
	return tail->get();
}

/**
 * Indexes list numerically by moving an iterator forward to that index.
 */

template <class T>
T& List<T>::operator[](std::size_t idx) {

	/**
	 * Move iterator forward.
	 */

	ListIterator<T> HeadIterator = begin();
	for (std::size_t i = 0; i < idx; ++i) {
		++HeadIterator;
	}

	/**
	 * Return contents.
	 */

	return *HeadIterator;
}

/**
 * Performs a linear search of the list for its contents.
 */

template <class T>
ListIterator<T> List<T>::find(T& data) {

	/**
	 * Create an iterator, loop through O(N) elements. If anything is found,
	 * return the iterator there. Otherwise return a null pointer.
	 */

	ListIterator<T> Iterator = begin();
	for (std::size_t i = 0; i < count; ++i) {
		++Iterator;
		if (*Iterator == data) return Iterator;
	}

	return ListIterator<T>();
}

/**
 * Returns size of list. Self explanatory.
 */

template <class T>
inline std::size_t List<T>::size(void) {
	return count;
}

#endif /* LIST_H_ */
