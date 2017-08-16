#pragma once

#include "block.h"

namespace Blocks {

class StringVariable : public Eventable {
public:
	StringVariable() {}
	StringVariable(const string& initial_value) : data_(initial_value) {}

	void setValue(const string& value) { data_ = value; }
	const string& value() const { return data_; }

	void event(BlockEvent /*e*/) override {} // TODO: Any events here?
	// Actions to set data, etc.
	void action(BlockAction a) override; // TODO

protected:
	string data_;
};

class IntVariable : public Eventable {
public:
	IntVariable() {}
	IntVariable(const int32_t initial_value) : data_(initial_value) {}

	const string& toString();

	void inc() { data_++; updated_ = true; }
	void dec() { data_--; updated_ = true; }
	void add(const int32_t value) { data_ += value; updated_ = true; }
	void sub(const int32_t value) { data_ -= value; updated_ = true; }
	void mult(const int32_t value) { data_ *= value; updated_ = true; }
	void div(const int32_t value) { data_ /= value; updated_ = true; }

	void setValue(const int32_t value) { data_ = value; updated_ = true; }
	int32_t value() const { return data_; }

	void event(BlockEvent /*e*/) override {} // TODO: Any events here?
	// Actions to set data, increment, decrement, etc.
	void action(BlockAction a) override;

protected:
	string to_string_data_;
	int32_t data_ = 0;
	bool updated_ = true;
};

// TODO: Possibly also Int64, etc.

} // namespace Blocks
