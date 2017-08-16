#pragma once

#include <vector>

namespace arc {

// TODO: Make these actual classes?

// Slot vector functions
template<typename T>
void deleteallslotvector(std::vector<T*>& items) {
	const size_t len = items.size();
	for (size_t i = 0; i < len; i++) {
		if (items[i] != nullptr) {
			delete items[i];
			items[i] = nullptr;
		}
	}
}

template<typename T>
T& addtoslotvector(std::vector<T*>& items, T* element) {
	const size_t len = items.size();
	for (size_t i = 0; i < len; i++) {
		if (items[i] == nullptr) {
			items[i] = element;
			return *element;
		}
	}
	items.push_back(element);
	return *element;
}

template<typename T>
size_t addtoslotvectorindex(std::vector<T*>& items, T* element) {
	const size_t len = items.size();
	for (size_t i = 0; i < len; i++) {
		if (items[i] == nullptr) {
			items[i] = element;
			return i;
		}
	}
	items.push_back(element);
	return len;
}

template<typename T>
void deletefromslotvector(std::vector<T*>& items, T* element, bool delete_item = true) {
	const size_t len = items.size();
	for (size_t i = 0; i < len; i++) {
		if (items[i] == element) {
			if (delete_item) {
				delete items[i];
			}
			items[i] = nullptr;
			return;
		}
	}
}

template<typename T>
void addtozerovector(std::vector<T>& items, T element) {
	const size_t len = items.size();
	for (size_t i = 0; i < len; i++) {
		if (items[i] == 0) {
			items[i] = element;
			return;
		}
	}
	items.push_back(element);
	return;
}

template<typename T>
void removefromzerovector(std::vector<T>& items, T element) {
	const size_t len = items.size();
	for (size_t i = 0; i < len; i++) {
		if (items[i] == element) {
			if (i == len - 1) {
				items.pop_back();
				return;
			} else {
				items[i] = 0;
				return;
			}
		}
	}
}

} // namespace arc
