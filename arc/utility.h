#pragma once

#include <cstdint>
#include <cstddef>

#define strequals(str1, str2) (strcmp(str1, str2) == 0)

template<typename T>
inline T rounduptomultiple(T value, T block_size) {
	T r = value % block_size;
	if (r == 0) {
		return value;
	} else {
		return value + (block_size - r);
	}
}

template <typename T>
inline void deleteelements(T* array, std::size_t len) {
	for (std::size_t i = 0; i < len; i++) {
		if (array[i] != nullptr) {
			delete array[i];
		}
	}
	delete[] array;
}

// Swaps n bytes of memory between ptr1 and ptr2, guaranteed to work even if both ranges overlap.
void memswap(void* ptr1, void* ptr2, std::size_t n) ;

// Same as above, but the ranges must not overlap!
void memswapdisjoint(void* ptr1, void* ptr2, std::size_t n);

template<class T>
inline const T& min(const T& a, const T& b) {
	return (a < b) ? a : b;
}

template<class T>
inline const T& max(const T& a, const T& b) {
	return (a < b) ? b : a;
}

template<class T>
inline const T& max(const T& a, const T& b, const T& c) {
	const T& m1 = (a < b) ? b : a;
	return (m1 < c) ? c : m1;
}
