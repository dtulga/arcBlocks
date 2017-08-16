#pragma once

#include "arc.h"

namespace arc {

template <typename T>
struct memory_arcinternal {
	T* data_ = nullptr;
	size_t len_ = 0;
	size_t capacity_ = 0; // If the data is owned then capacity_ > 0
	mutable uint32_t ref_count_ = 0; // Needs to be mutable so this can be incremented when copied.
	
	bool owned() const { return capacity_ > 0; }
};

// const reference: memory -> const data
// mutable unowned: memory -> data (doesn't delete the data, but does delete the memory object upon ref == 0)
// mutable owned: memory -> data (deletes the data too)

// This class handles all reference counting and data retrieval/allocation/deallocation,
// and is safe to pass around/copy/move at will.
// It also performs copy-on-write for the array-like functions.
// Note that memory doesn't perform any range-checking for access, only on write/copy.
// Use arc::array for range-checked memory.
// Also note that side-effect-causing objects cannot be stored in this class without proper checking!
// (Objects that allocate other memory/resources/etc.)
template <typename T>
class memory {
public:
	memory() {} // Uses the default args below.
	explicit memory(const size_t len); // Allocates the data (default values).
	memory(const T& value, const size_t len); // Allocates len of value items.
	memory(T* data, const size_t len, const bool own);
	memory(const T* data, const size_t len) : data_(data), len_(len) {} // Doesn't take ownership (or create a memory_arcinternal object)

	memory(const memory& other) { // copy constructor
		copy_ref_from(other);
	}
	memory(memory&& other) { // move constructor
		copy_ref_from(other);
		other.remove_ref();
	}

	memory& operator=(const memory& other) { // copy assignment
	    if (this != &other) { // self-assignment check
	        copy_ref_from(other); // Also removes this ref
	    }
	    return *this;
	}
	memory& operator=(memory&& other) { // move assignment
	    copy_ref_from(other); // Also removes this ref
	    other.remove_ref(); // leave moved-from in valid state
	    return *this;
	}

	~memory() {	remove_ref(); }

	const T* data() const;
	const T* get() const { return data(); }

	size_t len() const;
	size_t length() const { return len(); }
	size_t size() const { return len(); }
	bool empty() const { return len() == 0; }

	bool owned() const;
	size_t capacity() const; // Returns 0 is not owned.
	uint32_t ref_count() const;

	T* mutable_data();
	T* mutable_get() { return mutable_data(); }

	// TODO: Optional: prepend (mem), insert (mem)

	memory& append(const T& value); // amortized O(1) when unreserved, and returns this so it can be chained.
	memory& append(T&& value);
	memory& append(const memory& other);
	memory& append(memory&& other);
	T pop();

	memory& insert(const size_t i, const T& value); // O(n) since elements are copied/moved. (Except at end, which is the same as append)
	memory& insert(const size_t i, T&& value);
	memory& prepend(const T& value) { return insert(0, value); }
	memory& prepend(T&& value) { return insert(0, value); }
	T remove(const size_t i);

	void swap(const size_t i, const size_t j);

	void swap_with(memory& other);
	void copy_from(const memory& other) { copy_ref_from(other); }

	// WARNING: These return references that must not be used after this memory is deallocated! //

	const T& first() const;
	memory first(const size_t count) const;
	const T& last() const;
	memory last(const size_t count) const;

	memory sub(const size_t start, size_t end = -1) const; // Returns [start, end) (Default returns to the end, same as passing NotFound.)

	const T& at(const size_t i) const;
	const T& operator[] (const size_t i) const;

	// Warning: These two can cause copies! (Only performance, not bugs.)
	T& mutable_at(const size_t i);
	T& operator[] (const size_t i);

	// End reference return section //

	bool operator==(const memory& other) const;
	bool operator!=(const memory& other) const { return !(operator==(other)); }
	bool operator<(const memory& rhs) const;
	bool operator>(const memory& rhs) const;

	void sort(); // Ascending
	void reverse_sort(); // Descending

	// TODO:
	//memory<memory<T>> split(const T& splitter) const;
	//static memory<T> split(const T& value, const T& splitter);
	T join(const T& joiner) const;

	// indexOf returns NOT_FOUND (== -1) when not found.
	size_t indexOf(const T& e, const size_t search_start = 0) const; // O(n)
	bool contains(const T& e, const size_t search_start = 0) const; // O(n)
	size_t indexOf(const memory& search, const size_t search_start = 0) const; // O(n*m)
	bool contains(const memory& search, const size_t search_start = 0) const; // O(n*m)
	
	// These are search_start elements, counting from the end to the beginning:
	size_t lastIndexOf(const T& e, const size_t search_start = 0) const; // O(n)
	size_t lastIndexOf(const memory& search, const size_t search_start = 0) const; // O(n*m)

	size_t count(const T& e, const size_t search_start = 0) const;
	size_t count(const memory& search, const size_t search_start = 0) const; // Non-overlapping.

	memory<size_t> indexesOf(const T& e, const size_t search_start = 0) const;
	memory<size_t> indexesOf(const memory& search, const size_t search_start = 0) const; // Non-overlapping.

	bool prefix(const memory& compare) const;
	bool suffix(const memory& compare) const;

	void fill(const T& e, const size_t start = 0, const size_t end = -1); // Fill elements [start, end) with e
	void assign(const T& e, const size_t count = -1); // Assign the whole block to count length copies of e (auto-resize/expand)

	//void resize(const size_t new_size); // TODO: Will allocate new space if necessary

	void reserve(const size_t reserve_size);
	void clear() { remove_ref(); }

	static const size_t NotFound = -1;

protected:
	bool is_write_ready();

	void expand_to_at_least(const size_t new_len);

	void prepare_for_write(size_t new_capacity = 0);
	size_t prepare_for_insert_at(const size_t i); // Returns the new_len

	// Copies the reference from the other memory object. (Also handles const data_, and requires that this != &other)
	void copy_ref_from(const memory& other);

	void remove_ref();

	memory_arcinternal<T>* mem_ = nullptr;
	const T* data_ = nullptr;
	size_t len_ = 0;
};

template <typename T>
inline bool memory<T>::is_write_ready() {
	return mem_ != nullptr && mem_->owned() == true && mem_->ref_count_ == 1;
}

} // namespace arc

// Required for templates to work properly. :/
#include "memory.tpp"
