//include "memory.h"
// Template implementation - included by the header file.

#include <algorithm>

#define MEMORY_EXPANSION_FACTOR 2 /* For amortized O(1) expansion */

namespace arc {

template <typename T>
memory<T>::memory(const size_t len) { // Allocates the data (default values).
	mem_ = new memory_arcinternal<T>();
	size_t cap = max(len, std::size_t{ 1 });
	mem_->data_ = (T*) malloc(cap * sizeof(T));
	if (mem_->data_ == nullptr) {
		puts("malloc failed in memory constructor"); exit(1);
	}
	for (size_t i = 0; i < len; i++) {
		new(&(mem_->data_[i])) T();
	}
	mem_->len_ = len;
	mem_->capacity_ = cap;
	mem_->ref_count_ = 1;
}

template <typename T>
memory<T>::memory(const T& value, const size_t len) { // Allocates len of value items.
	mem_ = new memory_arcinternal<T>();
	size_t cap = max(len, std::size_t{ 1 });
	mem_->data_ = (T*) malloc(cap * sizeof(T));
	if (mem_->data_ == nullptr) {
		puts("malloc failed in memory constructor"); exit(1);
	}
	for (size_t i = 0; i < len; i++) {
		new(&(mem_->data_[i])) T(value);
	}
	mem_->len_ = len;
	mem_->capacity_ = cap;
	mem_->ref_count_ = 1;
}

template <typename T>
memory<T>::memory(T* data, const size_t len, const bool own) {
	mem_ = new memory_arcinternal<T>();
	mem_->data_ = data;
	mem_->len_ = len;
	mem_->capacity_ = own ? max(len, std::size_t{ 1 }) : 0;
	mem_->ref_count_ = 1;
}

template <typename T>
const T* memory<T>::data() const {
	if (mem_ != nullptr) {
		return mem_->data_;
	} else {
		return data_;
	}
}

template <typename T>
size_t memory<T>::len() const {
	if (mem_ != nullptr) {
		return mem_->len_;
	} else {
		return len_;
	}
}

template <typename T>
bool memory<T>::owned() const {
	if (mem_ != nullptr) {
		return mem_->owned();
	}
	return false;
}

template <typename T>
size_t memory<T>::capacity() const {
	if (mem_ != nullptr) {
		return mem_->capacity_;
	}
	return 0;
}

template <typename T>
uint32_t memory<T>::ref_count() const {
	if (mem_ != nullptr) {
		return mem_->ref_count_;
	}
	return 0;
}

template <typename T>
T* memory<T>::mutable_data() {
	prepare_for_write();
	return mem_->data_;
}

template <typename T>
void memory<T>::reserve(const size_t reserve_size) {
	if (!is_write_ready()) {
		size_t new_capacity = max(len(), reserve_size, std::size_t{ 1 });
		prepare_for_write(new_capacity);
	} else if (mem_->capacity_ < reserve_size) {
		void* new_data = realloc(mem_->data_, reserve_size * sizeof(T));
		if (new_data == nullptr) {
			puts("realloc failed in memory reserve"); exit(1);
		}
		mem_->data_ = (T*) new_data;
		mem_->capacity_ = reserve_size;
	}
	// Note that the len_ and the stored data never changes in this function.
}

// Uses the current length if count == -1
template <typename T>
void memory<T>::assign(const T& e, const size_t count) {
	size_t new_len;
	const size_t use_cl = -1;
	const bool data_to_remove = is_write_ready();
	if (count == use_cl) {
		if (!data_to_remove) {
			prepare_for_write();
		}
		new_len = len();
	} else {
		reserve(count);
		new_len = count;
	}

	if (data_to_remove) {
		const size_t ol = mem_->len_;
		for (size_t i = 0; i < ol; i++) {
			mem_->data_[i].~T();
		}
	}

	mem_->len_ = new_len;

	for (size_t i = 0; i < new_len; i++) {
		new (&(mem_->data_[i])) T(e);
	}
}

template <typename T>
memory<T>& memory<T>::append(const T& value) {
	expand_to_at_least(len()+1);
	// Construct object in-place into buffer
	new (&(mem_->data_[mem_->len_])) T(value);
	mem_->len_++;

	return *this;
}

template <typename T>
memory<T>& memory<T>::append(T&& value) {
	expand_to_at_least(len()+1);
	// Construct object in-place into buffer
	new (&(mem_->data_[mem_->len_])) T(std::move(value)); // Uses the move constructor
	mem_->len_++;

	return *this;
}

template <typename T>
memory<T>& memory<T>::append(const memory<T>& other) {
	size_t other_len = other.len();
	if (other_len == 0) {
		return *this; // Nothing needed
	}
	size_t len = this->len();
	size_t new_len = len + other_len;
	expand_to_at_least(new_len);

	// Copy Data
	//memcpy/memove(dest, src, n)
	// Use memmove as regions may overlap (i.e. sub reference appended)
	memmove(&(mem_->data_[len]), other.mem_ == nullptr ? other.data_ : other.mem_->data_, sizeof(T) * other_len);
	
	mem_->len_ = new_len;
	
	return *this;
}

template <typename T>
memory<T>& memory<T>::append(memory<T>&& other) {
	size_t other_len = other.len();
	if (other_len == 0) {
		return *this; // Nothing needed
	}
	size_t len = this->len();
	if (len == 0) {
		// Special case move-over
		swap_with(other);
		return *this; // Done
	}
	size_t new_len = len + other_len;
	expand_to_at_least(new_len);

	// Copy Data
	//memcpy/memove(dest, src, n)
	// Use memmove as regions may overlap (i.e. sub reference appended)
	memmove(&(mem_->data_[len]), other.mem_ == nullptr ? other.data_ : other.mem_->data_, sizeof(T) * other_len);
	
	mem_->len_ = new_len;

	other.remove_ref();

	return *this;
}

template <typename T>
void memory<T>::expand_to_at_least(const size_t new_len) {
	if (!is_write_ready()) {
		size_t new_capacity = max(max(len(), std::size_t{ 1 }) * MEMORY_EXPANSION_FACTOR, new_len);
		prepare_for_write(new_capacity);
	} else if (mem_->capacity_ < new_len) {
		size_t new_capacity = max(mem_->capacity_ * MEMORY_EXPANSION_FACTOR, new_len);
		reserve(new_capacity);
	}
}

template <typename T>
T memory<T>::pop() {
	if (empty()) {
		return T();
	}
	prepare_for_write();
	mem_->len_--;
	T tmp(mem_->data_[mem_->len_]);
	mem_->data_[mem_->len_].~T(); // Destroy the object (memory still stays allocated, so capacity doesn't change)
	return tmp;
}

// If i is greater than len then add default values up to that index.
template <typename T>
memory<T>& memory<T>::insert(const size_t i, const T& value) { // O(n) since elements are copied/moved. (Except at end, which is the same as append)
	size_t len = this->len();
	if (i == len) {
		return append(value);
	}

	prepare_for_insert_at(i);

	// Construct object in-place into buffer
	new (&(mem_->data_[i])) T(value);

	return *this;
}

template <typename T>
memory<T>& memory<T>::insert(const size_t i, T&& value) { // O(n) since elements are copied/moved. (Except at end, which is the same as append)
	size_t len = this->len();
	if (i == len) {
		return append(std::move(value));
	}

	prepare_for_insert_at(i);

	// Construct object in-place into buffer
	new (&(mem_->data_[i])) T(value); // Uses the move constructor

	return *this;
}

template <typename T>
size_t memory<T>::prepare_for_insert_at(const size_t i) {
	size_t new_len = max(mem_->len_, i) + 1;
	expand_to_at_least(new_len);

	if (i > len()) {
		// Add new default values
		for (size_t j = len(); j < i; j++) {
			new (&(mem_->data_[j])) T();
		}
	} else { // i < len (as i == len is covered above)
		// Shift elements
		// memove(dest, src, n)
		memmove(&(mem_->data_[i+1]), &(mem_->data_[i]), sizeof(T) * (len() - i));
	}

	mem_->len_ = new_len;

	return new_len;
}

template <typename T>
T memory<T>::remove(const size_t i) {
	if (i >= len()) {
		return T();
	}
	if (i == len() - 1) {
		return pop();
	}
	prepare_for_write();
	T tmp(mem_->data_[i]);
	mem_->data_[i].~T();
	mem_->len_--;

	// Use memmove here and above to avoid bugs and unneccessary constructor/destructor calls.
	memmove(&(mem_->data_[i]), &(mem_->data_[i+1]), sizeof(T) * (mem_->len_ - i)); // len_ already decremented.

	return tmp;
}

template <typename T>
void memory<T>::swap(const size_t i, const size_t j) {
	if (i == j || i >= len() || j >= len()) { // TODO: Add in a warning/error here, or just in array/string?
		return;
	}
	prepare_for_write();
	memswapdisjoint(&(mem_->data_[i]), &(mem_->data_[j]), sizeof(T)); // Also prevents unneccessary copies and constructor/destructor calls.
}

// No allocations or reference changes needed as it is a equivalent trade.
template <typename T>
void memory<T>::swap_with(memory<T>& other) {
	auto tmem = other.mem_;
	auto tdata = other.data_;
	auto tlen = other.len_;
	other.mem_ = mem_;
	other.data_ = data_;
	other.len_ = len_;
	mem_ = tmem;
	data_ = tdata;
	len_ = tlen;
}

template <typename T>
const T& memory<T>::first() const {
	if (mem_ != nullptr) {
		return mem_->data_[0];
	} else {
		return data_[0];
	}
}

template <typename T>
const T& memory<T>::last() const {
	if (mem_ != nullptr) {
		return mem_->data_[mem_->len_-1];
	} else {
		return data_[len_-1];
	}
}

template <typename T>
memory<T> memory<T>::first(const size_t count) const {
	if (mem_ != nullptr) {
		if (count > mem_->len_) {
			return memory();
		}
		return memory(mem_->data_, count);
	} else {
		if (count > len_ || data_ == nullptr) {
			return memory();
		}
		return memory(data_, count);
	}
}

template <typename T>
memory<T> memory<T>::last(const size_t count) const {
	if (mem_ != nullptr) {
		if (count > mem_->len_) {
			return memory();
		}
		return memory(mem_->data_ + (mem_->len_ - count), count);
	} else {
		if (count > len_ || data_ == nullptr) {
			return memory();
		}
		return memory(data_ + (len_ - count), count);
	}
}

template <typename T>
memory<T> memory<T>::sub(const size_t start, size_t end) const { // Returns [start, end)
	if (end == NotFound) { // TODO: Warning about end > len() ?
		end = len();
	}
	if (mem_ != nullptr) {
		if (end > mem_->len_ || start >= end) {
			return memory();
		}
		return memory(mem_->data_ + start, end - start);
	} else {
		if (end > len_ || start >= end || data_ == nullptr) {
			return memory();
		}
		return memory(data_ + start, end - start);
	}
}

template <typename T>
bool memory<T>::operator==(const memory<T>& other) const {
	if (len() != other.len()) {
		return false;
	}
	for (size_t i = 0; i < len(); i++) {
		if (!(at(i) == other.at(i))) {
			return false;
		}
	}
	return true;
}

template <typename T>
bool memory<T>::operator<(const memory<T>& rhs) const {
	for (size_t i = 0; i < len() && i < rhs.len(); i++) {
		if (at(i) < rhs.at(i)) {
			return true;
		}
	}
	return len() < rhs.len();
}

template <typename T>
bool memory<T>::operator>(const memory<T>& rhs) const {
	for (size_t i = 0; i < len() && i < rhs.len(); i++) {
		if (at(i) > rhs.at(i)) {
			return true;
		}
	}
	return len() > rhs.len();
}

struct greater_sort {
    template<typename T>
    bool operator()(const T& a, const T& b) const { return a > b; }
};

template <typename T>
void memory<T>::sort() {
	if (len() > 1) {
		prepare_for_write();
		std::sort(std::begin(mem_->data_), std::end(mem_->data_));
	}
}

template <typename T>
void memory<T>::reverse_sort() {
	if (len() > 1) {
		prepare_for_write();
		std::sort(std::begin(mem_->data_), std::end(mem_->data_), greater_sort());
	}
}

template <typename T>
T memory<T>::join(const T& joiner) const {
	const size_t len = this->len();
	if (len == 0) {
		return T();
	} else if (len == 1) {
		return at(0);
	}
	const size_t j_len = joiner.len();
	T joined;
	size_t total_len = j_len * (len - 1);
	for (size_t i = 0; i < len; i++) {
		total_len += at(i).len();
	}
	joined.reserve(len);
	for (size_t i = 0; i < len - 1; i++) {
		joined.append(at(i));
		if (j_len > 0) {
			joined.append(joiner);
		}
	}
	joined.append(at(len - 1));
	return joined;
}

template <typename T>
size_t memory<T>::indexOf(const T& e, const size_t search_start) const { // O(n)
	for (size_t i = search_start; i < len(); i++) {
		if (at(i) == e) {
			return i;
		}
	}
	return NOT_FOUND;
}

template <typename T>
bool memory<T>::contains(const T& e, const size_t search_start) const { // O((n-m)*m)
	return indexOf(e, search_start) != NotFound;
}

template <typename T>
size_t memory<T>::indexOf(const memory<T>& search, const size_t search_start) const { // O((n-m)*m)
	const size_t len = this->len();
	const size_t s_len = search.len();
	if (len == 0 || s_len == 0 || search_start + s_len > len) {
		return NOT_FOUND;
	}
	for (size_t i = search_start; i + s_len <= len; i++) { // Not enough room left for a match.
		if (at(i) == search.at(0)) { // Possible match
			if (s_len == 1) {
				return i;
			}
			for (size_t j = 1; j < s_len; j++) {
				if (at(i+j) == search.at(j)) {
					// Matching so far.
					if (j + 1 == s_len) {
						return i;
					}
				} else {
					break; // Not a match.
				}
			}
		}
	}
	return NOT_FOUND;
}

template <typename T>
size_t memory<T>::lastIndexOf(const T& e, const size_t search_start) const { // O(n)
	const size_t len = this->len();
	if (len == 0 || search_start >= len) {
		return NOT_FOUND;
	}
	size_t i = len - 1 - search_start;
	while (true) {
		if (at(i) == e) {
			return i;
		}
		if (i == 0) {
			break;
		}
		i--;
	}
	return NOT_FOUND;
}

template <typename T>
size_t memory<T>::lastIndexOf(const memory& search, const size_t search_start) const { // O(n*m)
	const size_t len = this->len();
	const size_t s_len = search.len();
	if (len == 0 || s_len == 0 || search_start + s_len > len) {
		return NOT_FOUND;
	}
	size_t i = len - 1 - search_start;
	while (true) {
		if (at(i) == search.at(s_len - 1)) { // Possible match
			if (s_len == 1) {
				return i;
			}
			size_t j = s_len - 2;
			while (true) {
				if (at(i - (s_len - 1) + j) == search.at(j)) {
					// Matching so far.
					if (j == 0) {
						return i;
					}
				} else {
					break; // Not a match.
				}
				j--;
			}
		}
		if (i == s_len - 1) { // Not enough room left for a match.
			break;
		}
		i--;
	}
	return NOT_FOUND;
}

template <typename T>
bool memory<T>::contains(const memory<T>& search, const size_t search_start) const { // O(n*m)
	return indexOf(search, search_start) != NotFound;
}

template <typename T>
size_t memory<T>::count(const T& e, const size_t search_start) const {
	size_t n = 0;
	const size_t len = this->len();
	for (size_t i = search_start; i < len; i++) {
		if (at(i) == e) {
			n++;
		}
	}
	return n;
}

template <typename T>
size_t memory<T>::count(const memory<T>& search, const size_t search_start) const { // Non-overlapping.
	size_t n = 0;
	const size_t len = this->len();
	const size_t s_len = search.len();

	for (size_t x = search_start; x + s_len <= len; x += s_len) {
		x = indexOf(search, x);
		if (x == NotFound) {
			break;
		}
		n++;
	}
	return n;
}

template <typename T>
memory<size_t> memory<T>::indexesOf(const T& e, const size_t search_start) const {
	memory<size_t> indexes;
	const size_t len = this->len();
	for (size_t i = search_start; i < len; i++) {
		if (at(i) == e) {
			indexes.append(i);
		}
	}
	return indexes;
}

template <typename T>
memory<size_t> memory<T>::indexesOf(const memory<T>& search, const size_t search_start) const { // Non-overlapping.
	memory<size_t> indexes;
	const size_t len = this->len();
	const size_t s_len = search.len();

	for (size_t x = search_start; x + s_len <= len; x += s_len) {
		x = indexOf(search, x);
		if (x == NotFound) {
			break;
		}
		indexes.append(x);
	}
	return indexes;
}

template <typename T>
bool memory<T>::prefix(const memory<T>& compare) const {
	if (len() < compare.len()) {
		return false;
	}
	for (size_t i = 0; i < compare.len(); i++) {
		if (!(at(i) == compare.at(i))) {
			return false;
		}
	}
	return true;
}

template <typename T>
bool memory<T>::suffix(const memory<T>& compare) const {
	if (len() < compare.len()) {
		return false;
	}
	size_t i = len() - 1;
	size_t j = compare.len() - 1;
	while (j > 0) {
		if (!(at(i) == compare.at(j))) {
			return false;
		}
		i--;
		j--;
	}
	// i = 0+, j == 0
	return at(i) == compare.at(0);
}

template <typename T>
const T& memory<T>::at(const size_t i) const {
	if (mem_ != nullptr) {
		return mem_->data_[i];
	} else {
		return data_[i];
	}
}

template <typename T>
T& memory<T>::mutable_at(const size_t i) {
	prepare_for_write();
	return mem_->data_[i];
}

template <typename T>
T& memory<T>::operator[] (const size_t i) { // Warning: can cause copies! (Only performance, not bugs.)
	prepare_for_write();
	return mem_->data_[i];
}

template <typename T>
const T& memory<T>::operator[] (const size_t i) const {
	return at(i);
}

template <typename T>
void memory<T>::prepare_for_write(const size_t new_capacity) {
	if (is_write_ready()) {
		return; // Done
	}

	// Allocate Data
	memory_arcinternal<T>* tmp = new memory_arcinternal<T>();
	size_t len = this->len();
	size_t capacity = max(new_capacity, len, std::size_t{ 1 });

	tmp->data_ = (T*) malloc(capacity * sizeof(T));
	if (tmp->data_ == nullptr) {
		puts("malloc failed in memory prepare_for_write"); exit(1);
	}
	tmp->len_ = len;
	tmp->capacity_ = capacity;
	tmp->ref_count_ = 1;

	// Copy Data
	//memcpy/memove(dest, src, n)
	if (len > 0) {
		memcpy(tmp->data_, mem_ == nullptr ? data_ : mem_->data_, sizeof(T) * len); // memcpy okay to use as these regions are guaranteed to not overlap.
	}

	// Remove the old reference, and set the new one.
	remove_ref();
	mem_ = tmp;
}

// Copies the reference from the other memory object. (Also handles const data_, and requires that this != &other)
template <typename T>
void memory<T>::copy_ref_from(const memory<T>& other) {
	if (other.mem_ != nullptr) {
		if (mem_ == other.mem_) {
			return; // Same, done.
		}
		// The const_cast here is OK, as the other.mem_ is originally declared as non-const.
		// And the only part ever modified is the ref_count_, as if this is written to, a copy of all the data is modified before write.
		other.mem_->ref_count_++; // Do this first to ensure that the memory is not freed in case we're copying to the same or an overlapping block.
	}
	remove_ref(); // Remove any existing reference from this object.
	if (other.mem_ != nullptr) {
		mem_ = (const_cast <memory_arcinternal<T>*> (other.mem_) );
	} else {
		data_ = other.data_;
		len_ = other.len_;
	}
}

template <typename T>
void memory<T>::remove_ref() {
	if (mem_ != nullptr) {
		mem_->ref_count_--;
		if (mem_->ref_count_ == 0) {
			if (mem_->owned()) {
				for (size_t j = 0; j < mem_->len_; j++) {
					mem_->data_[j].~T(); // Destruct all objects stored here.
				}
				free(mem_->data_);
			}
			delete mem_;
		}
		mem_ = nullptr; // Even if still allocated, this object no longer holds the reference once removed.
	}
	data_ = nullptr;
	len_ = 0;
}

} // namespace arc
