#pragma once

#include "array.h"
#include "string.h"

namespace arc {

class args {
public:
	args(const int argc, char* argv[]); // Pass directly from main.

	const string& cmd() const { return cmd_; }
	// Arg at index:
	const string& at(const size_t i) const;
	const string& operator[] (const size_t i) const { return at(i); }
	// Arg in the form of --arg value or -r value (empty string for short only, r = 0 for long only)
	// TODO: Unicode short args.
	const string& arg(const string& l, const unsigned char s = 0) const;
	const string& arg(const char* l, const unsigned char s = 0) const { return arg(string(l), s); }
	const string& operator[] (const string& l) const { return arg(l, 0); }
	const string& operator[] (const char* l) const { return arg(l, 0); }

	size_t indexOf(const string& l, const unsigned char s = 0) const;
	bool present(const string& l, const unsigned char s = 0) const { return indexOf(l, s) != NotFound; }
	bool contains(const string& l, const unsigned char s = 0) const { return present(l, s); }

	size_t len() const { return args_.len(); }
	size_t length() const { return args_.len(); }
	size_t size() const { return args_.len(); }

	const array<string>& toArray() const { return args_; }

protected:
	arc::array<string> args_; // argv[1]+
	string cmd_; // argv[0]
	const string empty_arg_;
	const size_t NotFound = -1;
};

} // namespace arc
