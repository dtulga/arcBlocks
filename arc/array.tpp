//include "array.h"
// Template implementation - included by the header file.

namespace arc {

template <typename T>
const T& array<T>::first() const {
	if (memory<T>::len() == 0) {
		throw std::out_of_range("array first called on empty array");
	}
	return memory<T>::first();
}

template <typename T>
const T& array<T>::last() const {
	if (memory<T>::len() == 0) {
		throw std::out_of_range("array last called on empty array");
	}
	return memory<T>::last();
}

template <typename T>
const T& array<T>::at(const size_t i) const {
	if (i >= memory<T>::len()) {
		throw std::out_of_range("array access with at out of range");
	}
	return memory<T>::at(i);
}

template <typename T>
T& array<T>::operator[] (const size_t i) { // Warning: can cause copies! (Only performance, not bugs.)
	if (i >= memory<T>::len()) {
		throw std::out_of_range("array access with operator[] out of range");
	}
	return memory<T>::mutable_at(i);
}

template <typename T>
const T& array<T>::operator[] (const size_t i) const {
	if (i >= memory<T>::len()) {
		throw std::out_of_range("array access with const operator[] out of range");
	}
	return memory<T>::at(i);
}

template <typename T>
T& array<T>::mutable_at(const size_t i) {
	if (i >= memory<T>::len()) {
		throw std::out_of_range("array access with mutable_at out of range");
	}
	return memory<T>::mutable_at(i);
}

// Only valid for T of memory/size types.
template <typename T>
size_t array<T>::combined_len() {
	const size_t len = memory<T>::len();
	size_t t_len = 0;
	for (size_t i = 0; i < len; i++) {
		t_len += memory<T>::at(i).size(); // So this works with std::vector etc. as well.
	}
	return t_len;
}

} // namespace arc
