#pragma once

#include <climits>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <cstdlib>

#include <stdexcept>

#include "utility.h"

#define B_0 0x0
// Values from 0x1 - 0xFF are for specific variants of a type.
#define B_1 0x100
#define B_2 0x200
#define B_3 0x400
#define B_4 0x800
#define B_5 0x1000
#define B_6 0x2000
#define B_7 0x4000
#define B_8 0x8000
// ^ Limit of uint16_t
#define B_9 0x10000
#define B_10 0x20000
#define B_11 0x40000
#define B_12 0x80000
#define B_13 0x100000
#define B_14 0x200000
#define B_15 0x400000
#define B_16 0x800000

#define NOT_FOUND -1

#define DELETE_COPY_AND_ASSIGN(class_name) class_name (const class_name &) = delete; class_name & operator=(const class_name &) = delete

using std::size_t;
using std::int8_t;
using std::int16_t;
using std::int32_t;
using std::int64_t;
using std::uint8_t;
using std::uint16_t;
using std::uint32_t;
using std::uint64_t;

namespace arc {

inline void print(const char* str) {
	fwrite(str, 1, strlen(str), stdout);
}

inline void print(const unsigned char* str) {
	fwrite(str, 1, strlen((char*) str), stdout);
}

inline void print(const char c) {
	putchar(c);
}

inline void print(const unsigned char c) {
	putchar(c);
}

inline void println(const char* str) {
	puts(str);
}

inline void println(const unsigned char* str) {
	puts((char*) str);
}

inline void println(const char c) {
	putchar(c);
	putchar('\n');
}

inline void println(const unsigned char c) {
	putchar(c);
	putchar('\n');
}

template <typename T0, typename T1>
struct multireturn2 {
	multireturn2(T0 val0, T1 val1) : item0(val0), item1(val1) {}
	T0 item0;
	T1 item1;
};

template <typename T0, typename T1, typename T2>
struct multireturn3 {
	multireturn3(T0 val0, T1 val1, T2 val2) : item0(val0), item1(val1), item2(val2) {}
	T0 item0;
	T1 item1;
	T1 item2;
};

// TODO: multireturn4+?

} // namespace arc
