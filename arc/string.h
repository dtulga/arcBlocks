#pragma once

#include "array.h"

namespace arc {

#define STRMAX_UINT64 21 /* 18446744073709551615 */
#define STRMAX_UINT32 11 /* 4294967295 */
#define STRMAX_UINT16 6 /* 65535 */
#define STRMAX_UINT8 4 /* 255 */
#define STRMAX_INT64 21 /* -9223372036854775808 */
#define STRMAX_INT32 12 /* -2147483648 */
#define STRMAX_INT16 7 /* -32768 */
#define STRMAX_INT8 5 /* -128 */


// Specialization of array for strings.
class string : public array<unsigned char> {
public:
	string() : array<unsigned char>() {} // Uses the default args below.

	string(const char c, const size_t len) : array<unsigned char>((unsigned char) c, len) {}
	string(const unsigned char c, const size_t len) : array<unsigned char>(c, len) {}
	string(const char* data, const size_t len) : array<unsigned char>((unsigned char*) data, len) {}
	string(const unsigned char* data, const size_t len) : array<unsigned char>(data, len) {}
	// Not explicit, as these can be used for conversions from C strings.
	string(const char * data) : array<unsigned char>((unsigned char*) data, strlen(data)) {}
	string(const unsigned char * data) : array<unsigned char>(data, strlen((char*) data)) {}

	using array<unsigned char>::array; // Copy the rest of the normal constructors.

	string(const string& other) : array<unsigned char>(other) {} // copy constructor
	string(const array<unsigned char>& other) : array<unsigned char>(other) {} // copy constructor
	string(const memory<unsigned char>& other) : array<unsigned char>(other) {} // copy constructor
	string(string&& other) : array<unsigned char>(std::move(other)) {} // move constructor
	string(array<unsigned char>&& other) : array<unsigned char>(std::move(other)) {} // move constructor
	string(memory<unsigned char>&& other) : array<unsigned char>(std::move(other)) {} // move constructor

	string& operator=(const string& other) { // copy assignment
		memory<unsigned char>::operator=(other);
	    return *this;
	}
	string& operator=(string&& other) { // move assignment
		memory<unsigned char>::operator=(std::move(other));
	    return *this;
	}
	string& operator=(const char* str) {
		memory<unsigned char>::clear(); // Removes any existing ref/data
		data_ = (unsigned char*) str;
		len_ = strlen(str);
		return *this;
	}
	string& operator=(const unsigned char* str) {
		memory<unsigned char>::clear(); // Removes any existing ref/data
		data_ = str;
		len_ = strlen((char*) str);
		return *this;
	}

	char* c_str(); // May cause copies due to need to append a null character (outside of len_ but in capacity_)

	using memory<unsigned char>::append;
	string& append(const char * str);
	string& append(const unsigned char * str);

	// These are re-referenced here so that string does bounds checking as well:

	const unsigned char& first() const { return array<unsigned char>::first(); }
	const unsigned char& last() const { return array<unsigned char>::last(); }

	const unsigned char& at(const size_t i) const { return array<unsigned char>::at(i); }
	const unsigned char& operator[] (const size_t i) const { return array<unsigned char>::at(i); }

	// Warning: These two can cause copies! (Only performance, not bugs.)
	unsigned char& mutable_at(const size_t i) { return array<unsigned char>::mutable_at(i); }
	unsigned char& operator[] (const size_t i) { return array<unsigned char>::mutable_at(i); }

	// indexOf returns NOT_FOUND (== -1) when not found.
	using memory<unsigned char>::indexOf;
	using memory<unsigned char>::contains;
	size_t indexOf(const char c, const size_t search_start = 0) const { return memory<unsigned char>::indexOf((unsigned char) c, search_start); } // O(n)
	bool contains(const char c, const size_t search_start = 0) const { return memory<unsigned char>::contains((unsigned char) c, search_start); } // O(n)
	size_t indexOf(const char* search, const size_t search_start = 0) const { return memory<unsigned char>::indexOf(string(search), search_start); } // O((n-m)*m)
	size_t indexOf(const unsigned char* search, const size_t search_start = 0) const { return memory<unsigned char>::indexOf(string(search), search_start); } // O((n-m)*m)
	bool contains(const char* search, const size_t search_start = 0) const { return memory<unsigned char>::contains(string(search), search_start); } // O((n-m)*m)
	bool contains(const unsigned char* search, const size_t search_start = 0) const { return memory<unsigned char>::contains(string(search), search_start); } // O((n-m)*m)

	// TODO: Adapters for lastIndexOf and indexesOf

	using memory<unsigned char>::prefix;
	using memory<unsigned char>::suffix;
	bool prefix(const memory& compare) const;
	bool suffix(const memory& compare) const;

	string& operator+=(const string& rhs);
	string& operator+=(const char* rhs);
	string& operator+=(const unsigned char* rhs);

	using memory<unsigned char>::operator==;
	using memory<unsigned char>::operator!=;
	using memory<unsigned char>::operator<;
	using memory<unsigned char>::operator>;
	bool operator==(const char* compare) const { return memory<unsigned char>::operator==(string(compare)); }
	bool operator!=(const char* compare) const { return memory<unsigned char>::operator!=(string(compare)); }
	bool operator<(const char* rhs) const { return memory<unsigned char>::operator<(string(rhs)); }
	bool operator>(const char* rhs) const { return memory<unsigned char>::operator>(string(rhs)); }
	bool operator==(const unsigned char* compare) const { return memory<unsigned char>::operator==(string(compare)); }
	bool operator!=(const unsigned char* compare) const { return memory<unsigned char>::operator!=(string(compare)); }
	bool operator<(const unsigned char* rhs) const { return memory<unsigned char>::operator<(string(rhs)); }
	bool operator>(const unsigned char* rhs) const { return memory<unsigned char>::operator>(string(rhs)); }

	void reset_to_empty(); // Preserves any memory allocated (if any)

	// Only to be called when this string has been manipulated by a c string function.
	void set_to_c_len();

	// Takes ownership of the already-allocated memory in buffer.
	void assign_c_str(char* buffer);

	static string itoa(const unsigned long long num);
	static string itoa(const unsigned long num);
	static string itoa(const unsigned int num);
	static string itoa(const unsigned short num);
	static string itoa(const unsigned char num);
	static string itoa(const long long num);
	static string itoa(const long num);
	static string itoa(const int num);
	static string itoa(const short num);
	static string itoa(const char num);
	// TODO: float, double, etc.

	array<string> split(const string& splitter) const; // Splitting with a blank string does nothing.
	array<string> split(const unsigned char* splitter) const { return split(string(splitter)); }
	array<string> split(const char* splitter) const { return split(string(splitter)); }
	array<string> split(const unsigned char splitter) const;
	array<string> split(const char splitter) const { return split((unsigned char) splitter); }
};

inline string& string::append(const char * str) {
	memory<unsigned char>::append(string(str));
	return *this;
}

inline string& string::append(const unsigned char * str) {
	memory<unsigned char>::append(string(str));
	return *this;
}

inline string& string::operator+=(const string& rhs) {
	memory<unsigned char>::append(rhs);
	return *this;
}

inline string& string::operator+=(const char* rhs) {
	return append(rhs);
}

inline string& string::operator+=(const unsigned char* rhs) {
	return append(rhs);
}

inline string operator+(string lhs, const string& rhs) {
	lhs += rhs;
	return lhs;
}

inline string operator+(string lhs, const char* rhs) {
	lhs += rhs;
	return lhs;
}

inline string operator+(string lhs, const unsigned char* rhs) {
	lhs += rhs;
	return lhs;
}

inline string operator+(const char* lhs, const string& rhs) {
	string str(lhs);
	str += rhs;
	return str;
}

inline string operator+(const unsigned char* lhs, const string& rhs) {
	string str(lhs);
	str += rhs;
	return str;
}

inline void print(const string& str) {
	fwrite(str.data(), 1, str.len(), stdout);
}

inline void println(const string& str) {
	fwrite(str.data(), 1, str.len(), stdout);
	putchar('\n');
}

} // namespace arc
