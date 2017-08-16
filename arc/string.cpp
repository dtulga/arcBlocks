#include "string.h"

namespace arc {

void string::reset_to_empty() { // Preserves any memory allocated (if any)
	if (memory<unsigned char>::is_write_ready()) {
		for (size_t i = 0; i < mem_->capacity_; i++) {
			mem_->data_[i] = '\0';
		}
		mem_->len_ = 0;
	} else {
		memory<unsigned char>::clear();
	}
}

// Only to be called when this string is manipulated by a c string function.
void string::set_to_c_len() {
	if (!memory<unsigned char>::is_write_ready()) {
		return; // No effect if not owned.
	}
	for (size_t i = 0; i < mem_->capacity_; i++) {
		if (mem_->data_[i] == '\0') {
			mem_->len_ = i;
			return;
		}
	}
	mem_->len_ = mem_->capacity_;
}

void string::assign_c_str(char* buffer) {
	memory<unsigned char>::clear();

	if (buffer == nullptr) {
		return;
	}

	mem_ = new memory_arcinternal<unsigned char>();
	size_t len = strlen(buffer);
	mem_->data_ = (unsigned char*) buffer;
	mem_->len_ = len;
	mem_->capacity_ = len + 1; // Plus the \0
	mem_->ref_count_ = 1;
}

string string::itoa(const unsigned long long num) {
	string buffer('\0', STRMAX_UINT64); // Initalizes an empty string
	snprintf( (char *) buffer.mutable_data(), STRMAX_UINT64, "%llu", num );
	buffer.set_to_c_len();
	return buffer;
}

string string::itoa(const unsigned long num) {
	string buffer('\0', STRMAX_UINT64); // Initalizes an empty string
	snprintf( (char *) buffer.mutable_data(), STRMAX_UINT64, "%lu", num );
	buffer.set_to_c_len();
	return buffer;
}

string string::itoa(const unsigned int num) {
	string buffer('\0', STRMAX_UINT32); // Initalizes an empty string
	snprintf( (char *) buffer.mutable_data(), STRMAX_UINT32, "%u", num );
	buffer.set_to_c_len();
	return buffer;
}

string string::itoa(const unsigned short num) {
	string buffer('\0', STRMAX_UINT16); // Initalizes an empty string
	snprintf( (char *) buffer.mutable_data(), STRMAX_UINT16, "%hu", num );
	buffer.set_to_c_len();
	return buffer;
}

string string::itoa(const unsigned char num) {
	string buffer('\0', STRMAX_UINT8); // Initalizes an empty string
	snprintf( (char *) buffer.mutable_data(), STRMAX_UINT8, "%hhu", num );
	buffer.set_to_c_len();
	return buffer;
}

// Signed:

string string::itoa(const long long num) {
	string buffer('\0', STRMAX_INT64); // Initalizes an empty string
	snprintf((char *)buffer.mutable_data(), STRMAX_INT64, "%lld", num);
	buffer.set_to_c_len();
	return buffer;
}

string string::itoa(const long num) {
	string buffer('\0', STRMAX_INT64); // Initalizes an empty string
	snprintf((char *)buffer.mutable_data(), STRMAX_INT64, "%ld", num);
	buffer.set_to_c_len();
	return buffer;
}

string string::itoa(const int num) {
	string buffer('\0', STRMAX_INT32); // Initalizes an empty string
	snprintf((char *)buffer.mutable_data(), STRMAX_INT32, "%d", num);
	buffer.set_to_c_len();
	return buffer;
}

string string::itoa(const short num) {
	string buffer('\0', STRMAX_INT16); // Initalizes an empty string
	snprintf((char *)buffer.mutable_data(), STRMAX_INT16, "%hd", num);
	buffer.set_to_c_len();
	return buffer;
}

string string::itoa(const char num) {
	string buffer('\0', STRMAX_INT8); // Initalizes an empty string
	snprintf((char *)buffer.mutable_data(), STRMAX_INT8, "%hhd", num);
	buffer.set_to_c_len();
	return buffer;
}

char* string::c_str() {
	reserve(len() + 1); // Guarantee space for the null character.
	mem_->data_[mem_->len_] = '\0'; // Write it, but don't increment len_.

	return (char*) mem_->data_;
}

array<string> string::split(const string& splitter) const {
	array<string> result;
	const size_t s_len = splitter.len();
	const memory<size_t>& indexes = indexesOf(splitter);
	const size_t idx_n = indexes.size();
	if (idx_n == 0) {
		result.reserve(1);
		result.append(*this);
		return result;
	}

	result.reserve(idx_n + 1);
	result.append(this->sub(0, indexes[0]));
	for (size_t i = 0; i < idx_n - 1; i++) {
		result.append(this->sub(indexes[i] + s_len, indexes[i + 1]));
	}
	result.append(this->sub(indexes[idx_n - 1] + s_len));
	return result;
}

array<string> string::split(const unsigned char splitter) const {
	array<string> result;
	const memory<size_t>& indexes = indexesOf(splitter);
	const size_t idx_n = indexes.size();
	if (idx_n == 0) {
		result.reserve(1);
		result.append(*this);
		return result;
	}

	result.reserve(idx_n + 1);
	result.append(this->sub(0, indexes[0]));
	for (size_t i = 0; i < idx_n - 1; i++) {
		result.append(this->sub(indexes[i] + 1, indexes[i + 1]));
	}
	result.append(this->sub(indexes[idx_n - 1] + 1));
	return result;
}

} // namespace arc
