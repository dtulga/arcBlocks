#pragma once

#include "memory.h"

namespace arc {

// Same as memory, but with range checking.
template <typename T>
class array : public memory<T> {
public:
	array() : memory<T>() {}

	using memory<T>::memory; // Copy the rest of the normal constructors

	array(const array& other) : memory<T>(other) {} // copy constructor
	array(const memory<T>& other) : memory<T>(other) {} // copy constructor
	array(array&& other) : memory<T>(std::move(other)) {} // move constructor
	array(memory<T>&& other) : memory<T>(std::move(other)) {} // move constructor

	array& operator=(const array& other) { // copy assignment
		memory<T>::operator=(other);
	    return *this;
	}
	array& operator=(array&& other) { // move assignment
		memory<T>::operator=(std::move(other));
	    return *this;
	}

	// WARNING: These return references that must not be used after this memory is deallocated! //
	// Also note that these can throw out-of-range exceptions! //

	const T& first() const;
	const T& last() const;

	const T& at(const size_t i) const;
	const T& operator[] (const size_t i) const;

	// Warning: These two can cause copies! (Only performance, not bugs.)
	T& mutable_at(const size_t i);
	T& operator[] (const size_t i);

	// End reference return and exception throwing section //

	// Only valid for T of memory/size types.
	size_t combined_len();
};

} // namespace arc

// Required for templates to work properly. :/
#include "array.tpp"
