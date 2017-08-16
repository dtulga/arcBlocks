#include "path.h"

#ifdef _WIN32
	#error "Windows directory/stat not implemented."
	#include <direct.h>
	#define SysCD _chdir
#else
	#include <sys/types.h>
	#include <sys/stat.h>
	#include <unistd.h>
	#define SysCD chdir
#endif

namespace arc {

// Gets: dir/dir/dir/file
string PathModule::File(const string& path) {
	size_t x = path.lastIndexOf(PathSeparator);
	if (x == string::NotFound) {
		return path;
	} else if (x == path.len() - 1) {
		return string();
	}
	return path.sub(x + 1);
}

string PathModule::Dir(const string& path) {
	size_t x = path.lastIndexOf(PathSeparator);
	if (x == string::NotFound) {
		return ARC_THIS_DIR_STR;
	} else if (x == path.len() - 1) {
		return path;
	}
	return path.sub(0, x + 1);
}

array<string> PathModule::Split(const string& path) { // Split on each path separator.
	return path.split(PathSeparator);
}

// TODO: Windows!
bool PathModule::IsAbsolute(const string& path) {
	if (path.len() == 0) {
		return false;
	} else if (path.at(0) == PathSeparator) {
		return true;
	}
	return false;
}

// The parent function should call string::reserve.
void PathModule::InternalJoin(string& joined, const string& to_join) const {
	const size_t j_len = joined.len();
	const size_t t_len = to_join.len();
	if (j_len == 0) {
		joined = to_join;
		return;
	} else if (t_len == 0) {
		return;
	}
	uint32_t x = 0;
	x += joined.last() == PathSeparator ? 1 : 0;
	x += to_join.first() == PathSeparator ? 1 : 0;
	if (x == 2) {
		joined.pop();
		joined.append(to_join);
	} else if (x == 1) {
		joined.append(to_join);
	} else { // x == 0
		joined.append(PathSeparator);
		joined.append(to_join);
	}
	return;
}

// Joins path fragments with OS-appropriate separators.
string PathModule::Join(const string& path1, const string& path2) const {
	string joined(path1);
	joined.reserve(path1.len() + path2.len() + 1);
	InternalJoin(joined, path2);
	return joined;
}

string PathModule::Join(const string& path1, const string& path2, const string& path3) const {
	string joined(path1);
	joined.reserve(path1.len() + path2.len() + path3.len() + 2);
	InternalJoin(joined, path2);
	InternalJoin(joined, path3);
	return joined;
}

string PathModule::Join(const string& path1, const string& path2, const string& path3, const string& path4) const {
	string joined(path1);
	joined.reserve(path1.len() + path2.len() + path3.len() + path4.len() + 3);
	InternalJoin(joined, path2);
	InternalJoin(joined, path3);
	InternalJoin(joined, path4);
	return joined;
}

bool PathModule::CD(const char* path) {
	if (SysCD(path) != 0) {
		perror("ERROR [PathModule] Failed to change the current working directory");
		return false;
	}
	working_dir_ = path;
	return true;
}

bool PathModule::CD(const string& path) {
	string new_working_dir = path;
	if (SysCD(new_working_dir.c_str()) != 0) {
		perror("ERROR [PathModule] Failed to change the current working directory");
		return false;
	}
	working_dir_ = new_working_dir;
	return true;
}

const string& PathModule::CurrentWorkingDir() {
	if (working_dir_.empty()) {
		#ifdef _WIN32
		char* buffer = _getcwd(nullptr, 0);
		if (buffer == nullptr) {
			perror("ERROR [PathModule] Failed to get the current working directory");
		} else {
			working_dir_.assign_c_str(buffer); // Takes ownership of the memory.
		}
		#else
		size_t path_max = pathconf(".", _PC_PATH_MAX);
		working_dir_.reserve(path_max);
		if (getcwd((char*) working_dir_.mutable_data(), path_max) == nullptr) {
			perror("ERROR [PathModule] Failed to get the current working directory");
		} else {
			working_dir_.set_to_c_len();
		}
		#endif
	}
	return working_dir_;
}

// Note that this also returns false if inaccessible.
bool PathModule::Exists(const string& path) {
	struct stat st;
	errno = 0;
	string cpath(path);

	if (stat(cpath.c_str(), &st) == -1) {
		if (errno != ENOENT && errno != ENOTDIR) {
			perror("ERROR [PathModule] Failed to stat path");
		}
		return false;
	}

	return true;
}

// Note that this also returns false if inaccessible.
bool PathModule::IsDir(const string& path) {
	struct stat st;
	errno = 0;
	string cpath(path);

	if (stat(cpath.c_str(), &st) == -1) {
		if (errno != ENOENT && errno != ENOTDIR) {
			perror("ERROR [DirModule] Failed to stat path");
		}
		return false;
	}

	return S_ISDIR(st.st_mode);
}

// Note that this also returns false if inaccessible.
bool PathModule::IsFile(const string& path) {
	struct stat st;
	errno = 0;
	string cpath(path);

	if (stat(cpath.c_str(), &st) == -1) {
		if (errno != ENOENT && errno != ENOTDIR) {
			perror("ERROR [DirModule] Failed to stat path");
		}
		return false;
	}

	return S_ISREG(st.st_mode);
}

} // namespace arc
