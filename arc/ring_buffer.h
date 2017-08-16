#pragma once

#include <cstdlib>
#include <thread>
#include <atomic>

namespace arc {

// WARNING //
// CURRENT RECOMMENDATIONS: sync, ring_buffer, memory, etc. only with plain data types and pointers! //
// Ideally all should work with complex types eventually. //

// WARNING: All calls must be used from the same thread! //
template<typename T>
class ring_buffer_local {
public:
	explicit ring_buffer_local(const uint32_t size) : write_(0), read_(0), size_(size) {
		data_ = (T*) malloc(sizeof(T) * size);
	}

	~ring_buffer_local() {
		free(data_);
	}

	bool trySend(const T& element) {
		const uint32_t cur_w = write_;
		const uint32_t next_w = next_index(cur_w);
		if (next_w != read_) { // Full
			data_[cur_w] = element;
			write_ = next_w;
			return true;
		} else {
			return false;
		}
	}

	bool tryRecv(T* element) {
		/*if (element == nullptr) {
			// TODO: Throw exception?
			return false;
		}*/
		const uint32_t cur_r = read_;
		if (cur_r != write_) { // Empty
			*element = data_[cur_r];
			read_ = next_index(cur_r);
			return true;
		} else {
			return false;
		}
	}

protected:
	uint32_t next_index(const uint32_t current) {
		uint32_t next = current + 1;
		if (next == size_) {
			next = 0;
		}
		return next;
	}

	// write_ == read_ is empty.
	uint32_t write_ = 0;
	uint32_t read_ = 0;
	const uint32_t size_;
	T* data_ = nullptr;
};

// TODO: ring_buffer_multi with sync instead of atomic, so multiple can read/write at the same time

// WARNING //
// Only call send from one thread, and recv from one other (can be the same or different, but 2 max) //
template<typename T>
class ring_buffer {
public:
	explicit ring_buffer(const uint32_t size) : size_(size) {
		write_.store(0);
		read_.store(0);
		data_ = (T*) malloc(sizeof(T) * size);
	}

	~ring_buffer() {
		free(data_);
	}

	void send(const T& element) {
		while (!trySend(element)) {
			std::this_thread::yield();
		}
	}

	bool trySend(const T& element) {
		const uint32_t cur_w = write_.load();
		const uint32_t next_w = next_index(cur_w);
		if (next_w != read_.load()) { // Full
			data_[cur_w] = element;
			write_.store(next_w);
			return true;
		} else {
			return false;
		}
	}

	T recv() {
		T element;
		while (!tryRecv(&element)) {
			std::this_thread::yield();
		}
		return element;
	}

	bool tryRecv(T* element) {
		/*if (element == nullptr) {
			// TODO: Throw exception?
			return false;
		}*/
		const uint32_t cur_r = read_.load();
		if (cur_r != write_.load()) { // Empty
			*element = data_[cur_r];
			read_.store(next_index(cur_r));
			return true;
		} else {
			return false;
		}
	}

protected:
	uint32_t next_index(const uint32_t current) {
		uint32_t next = current + 1;
		if (next == size_) {
			next = 0;
		}
		return next;
	}

	// write_ == read_ is empty.
	std::atomic<uint32_t> write_;
	std::atomic<uint32_t> read_;
	const uint32_t size_;
	T* data_ = nullptr;
};

} // namespace arc
