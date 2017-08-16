#include "args.h"

namespace arc {

args::args(const int argc, char* argv[]) {
	if (argc < 1) return;

	cmd_ = argv[0];
	if (argc == 1) return;

	const size_t len = argc - 1;
	args_.reserve(len);
	for (size_t i = 0; i < len; i++) {
		args_[i] = argv[i + 1];
	}
}

// Arg at index:
const string& args::at(const size_t i) const {
	if (i < args_.len()) {
		return args_.at(i);
	} else {
		return empty_arg_;
	}
}

// TODO: --arg as a value?
const string& args::arg(const string& l, const unsigned char s) const {
	size_t x = indexOf(l, s);
	if (x != NotFound && x + 1 < args_.len()) {
		return args_.at(x + 1);
	} else {
		return empty_arg_;
	}
}

// TODO: stacking short args like -spq
size_t args::indexOf(const string& l, const unsigned char s) const {
	if (l.len() > 0) {
		const size_t x = args_.indexOf("--" + l);
		if (x != NotFound) {
			return x;
		}
	}

	if (s != 0) {
		unsigned char sarg[3];
		sarg[0] = '-';
		sarg[1] = s;
		sarg[2] = '\0';
		return args_.indexOf(arc::string(sarg));
	}
	return NotFound;
}

} // namespace arc
