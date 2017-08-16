#include "utility.h"

#include <cstdio>
#include <cstring>
#include <cstdlib>

using std::size_t;

#define FATALPRINT(msg) puts(msg); exit(1)

// Swaps n bytes of memory between ptr1 and ptr2, guaranteed to work even if both ranges overlap.
void memswap(void* ptr1, void* ptr2, size_t n) {
	if (ptr1 == ptr2) {
		return;
	}
	if (ptr1 == nullptr || ptr2 == nullptr) {
		FATALPRINT("null pointer passed to memswap");
	}
	void* tmp = malloc(n);
	if (tmp == nullptr) {
		FATALPRINT("malloc failed in memswap");
	}
	//memcpy/memove(dest, src, n)
	memcpy(tmp, ptr1, n);
	memmove(ptr1, ptr2, n);
	memcpy(ptr2, tmp, n);

	free(tmp);
}

// Same as above, but the ranges must not overlap!
void memswapdisjoint(void* ptr1, void* ptr2, size_t n) {
	if (ptr1 == nullptr || ptr2 == nullptr) {
		FATALPRINT("null pointer passed to memswap");
	}
	void* tmp = malloc(n);
	if (tmp == nullptr) {
		FATALPRINT("malloc failed in memswap");
	}
	//memcpy/memove(dest, src, n)
	memcpy(tmp, ptr1, n);
	memcpy(ptr1, ptr2, n);
	memcpy(ptr2, tmp, n);

	free(tmp);
}
