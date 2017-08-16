#pragma once

#include "array.h"

namespace arc {

template <typename T>
class stack : public array<T> {
public:
	stack() : array<T>() {} // Uses the default args.

	using memory<T>::memory; // Copy the rest of the normal constructors.

	stack(const stack& other) : array<T>(other) {} // copy constructor
	stack(stack&& other) : array<T>(other) {} // move constructor

	stack& operator=(const stack& other) { // copy assignment
		memory<T>::operator=(other);
	    return *this;
	}
	stack& operator=(stack&& other) { // move assignment
		memory<T>::operator=(other);
	    return *this;
	}

	void push(const T& value) { array<T>::append(value); }
	void push(T&& value) { array<T>::append(value); }

	T& mutable_peek() { return array<T>::mutable_at(array<T>::len() - 1); }
	const T& peek() const { return array<T>::last(); }

	T& mutable_peek(size_t index) {
		return array<T>::mutable_at(array<T>::len() - 1 - index);
	}
	const T& peek(size_t index) const {
		return array<T>::at(array<T>::len() - 1 - index);
	}

	// These are re-referenced here so that stack does bounds checking as well:

	const T& first() const { return array<T>::first(); }
	const T& last() const { return array<T>::last(); }

	const T& at(const size_t i) const { return array<T>::at(i); }
	const T& operator[] (const size_t i) const { return array<T>::at(i); }

	// Warning: These two can cause copies! (Only performance, not bugs.)
	T& mutable_at(const size_t i) { return array<T>::mutable_at(i); }
	T& operator[] (const size_t i) { return array<T>::mutable_at(i); }
};

} // namespace arc
