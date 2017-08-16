#pragma once

#include <mutex>

#include "arc.h"

namespace arc {

template<typename T>
class with;

// TODO! memory/array/string synced //

// Synced access to a value across threads.
// Cannot be copied/assigned.
// TODO: Move ownership?
template<typename T>
class sync {
public:
	explicit sync(const T& value) : value_(value) {}
	explicit sync(T&& value) : value_(value) {}

	sync& operator=(const T& value) { return set(value); } // copy assignment
	sync& operator=(T&& value) { return set(value); } // move assignment

	sync& set(const T& value);
	sync& set(T&& value);

	T get();
	T operator()() { return get(); }
	operator T() { return get(); }

	bool operator==(const T& other) const;
	bool operator<(const T& rhs) const;
	bool operator>(const T& rhs) const;
	bool operator<=(const T& rhs) const;
	bool operator>=(const T& rhs) const;

	void lock() { mutex_.lock(); }
	bool try_lock() { return mutex_.try_lock(); }
	void unlock() { mutex_.unlock(); }

protected:
	T value_;
	std::mutex mutex_;

	friend class with<T>;

	DELETE_COPY_AND_ASSIGN(sync);
};

// Use like: with protected(variable); Then use protected instead of variable in the with block.
template<typename T>
class with {
	explicit with(sync<T>& protect) : protected_(protect) {
		protected_.lock();
		is_locked_ = true;
	}
	~with() { done(); }

	// This is so done() can be safely called in addition to the destructor.
	// But don't ever call any other functions on this once done() has been called!
	void done() {
		if (is_locked_) {
			protected_.unlock();
			is_locked_ = false;
		}
	}

	with& operator=(const T& value) { return set(value); } // copy assignment
	with& operator=(T&& value) { return set(value); } // move assignment

	with& set(const T& value) { protected_.value_ = value; return *this; }
	with& set(T&& value) { protected_.value_ = value; return *this; }

	T get() { return protected_.value_; }
	T operator()() { return get(); }

	bool operator==(const T& other) const { return protected_.value_ == other; }
	bool operator<(const T& rhs) const { return protected_.value_ < rhs; }
	bool operator>(const T& rhs) const { return protected_.value_ > rhs; }
	bool operator<=(const T& rhs) const { return protected_.value_ <= rhs; }
	bool operator>=(const T& rhs) const { return protected_.value_ >= rhs; }

protected:
	bool is_locked_ = false;
	sync<T>& protected_;

	DELETE_COPY_AND_ASSIGN(with);
};

} // namespace arc

// Required for templates to work properly. :/
#include "sync.tpp"
