//include "sync.h"
// Template implementation - included by the header file.

namespace arc {

template <typename T>
sync<T>& sync<T>::set(const T& value) {
	std::lock_guard<std::mutex> lock(mutex_); // Guarantees that the mutex is unlocked when out of scope, even with exceptions.
	value_ = value;
	return *this;
}

template <typename T>
sync<T>& sync<T>::set(T&& value) {
	std::lock_guard<std::mutex> lock(mutex_); // Guarantees that the mutex is unlocked when out of scope, even with exceptions.
	value_ = value;
	return *this;
}

template <typename T>
T sync<T>::get() {
	std::lock_guard<std::mutex> lock(mutex_);
	T ret(value_);
	return ret;
}

template <typename T>
bool sync<T>::operator==(const T& other) const {
	std::lock_guard<std::mutex> lock(mutex_);
	return value_ == other;
}

template <typename T>
bool sync<T>::operator<(const T& rhs) const {
	std::lock_guard<std::mutex> lock(mutex_);
	return value_ < rhs;
}

template <typename T>
bool sync<T>::operator>(const T& rhs) const {
	std::lock_guard<std::mutex> lock(mutex_);
	return value_ > rhs;
}

template <typename T>
bool sync<T>::operator<=(const T& rhs) const {
	std::lock_guard<std::mutex> lock(mutex_);
	return value_ <= rhs;
}

template <typename T>
bool sync<T>::operator>=(const T& rhs) const {
	std::lock_guard<std::mutex> lock(mutex_);
	return value_ >= rhs;
}

} // namespace arc
