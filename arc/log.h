#pragma once

#include "arc.h"
#include "string.h"

namespace arc { namespace log {

// TODO: Allow for redirecting to a logfile, etc.
/*
class LogModule {
public:
	LogModule() {}
*/

template<typename T1, typename T2>
void Info(const T1& class_func_name, const T2& message) {
	arc::print('[');
	arc::print(class_func_name);
	arc::print("] ");
	arc::println(message);
}

template<typename T1, typename T2>
void Warn(const T1& class_func_name, const T2& message) {
	arc::print("Warning [");
	arc::print(class_func_name);
	arc::print("] ");
	arc::println(message);
}

template<typename T1, typename T2>
void Error(const T1& class_func_name, const T2& message) {
	arc::print("ERROR [");
	arc::print(class_func_name);
	arc::print("] ");
	arc::println(message);
}

template<typename T1, typename T2>
void Fatal(const T1& class_func_name, const T2& message) {
	arc::print("FATAL ERROR [");
	arc::print(class_func_name);
	arc::print("] ");
	arc::println(message);
	exit(1);
}

template<typename T>
void Info(const T& message) {
	arc::println(message);
}

template<typename T>
void Warn(const T& message) {
	arc::print("Warning: ");
	arc::println(message);
}

template<typename T>
void Error(const T& message) {
	arc::print("ERROR: ");
	arc::println(message);
}

template<typename T>
void Fatal(const T& message) {
	arc::print("FATAL ERROR: ");
	arc::println(message);
	exit(1);
}

//} log;

} } // namespace arc::log
