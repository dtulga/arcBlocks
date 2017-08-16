#include "variable.h"

namespace Blocks {

const string& IntVariable::toString() {
	if (!updated_) {
		return to_string_data_;
	}

	if (to_string_data_.capacity() < STRMAX_INT32) {
		to_string_data_.assign('\0', STRMAX_INT32);
	}

	snprintf((char *)to_string_data_.mutable_data(), STRMAX_INT32, "%d", data_);
	to_string_data_.set_to_c_len();

	return to_string_data_;
}

void IntVariable::action(BlockAction a) {
	if (!a.is_var_int()) {
		return;
	}

	const int32_t val = a.var_int.value;

	switch (a.type) {
	case BlockAction::IntSetValue:
		setValue(val);
		break;
	case BlockAction::IntInc:
		inc();
		break;
	case BlockAction::IntDec:
		dec();
		break;
	case BlockAction::IntAdd:
		add(val);
		break;
	case BlockAction::IntSub:
		sub(val);
		break;
	case BlockAction::IntMult:
		mult(val);
		break;
	case BlockAction::IntDiv:
		div(val);
		break;
	}
}

} // namespace Blocks
