#include "dir.h"

#include "path.h"
#include "file.h"

#include <errno.h>

#ifdef _WIN32
	#error "Windows directory/stat not implemented."
	#include <windows.h>
	#include <direct.h>
	// TODO: WIN32 API Calls for Directories.
#else
	#include <sys/types.h>
	#include <sys/stat.h>
	#include <dirent.h>
	#include <unistd.h>
#endif

namespace arc {

// Lists the contents of the current working dir.
array<string> DirModule::List() const {
	return List(string("."));
}

// Lists the contents of the given dir.
array<string> DirModule::List(const string& dir) const {
	array<string> entries;
	errno = 0;
	string cdir(dir);
	DIR* d = opendir(cdir.c_str());
	if (d == nullptr) {
		perror("ERROR [DirModule] Failed to open directory");
		return entries;
	}

	while (true) {
		errno = 0;
		const auto* entry = readdir(d);
		if (entry == nullptr) {
			if (errno != 0) {
				perror("ERROR [DirModule] Failed to read directory entry");
			}
			break;
		}
		// Use d_type for the type of file/directory. (If needed)
		entries.append(string(entry->d_name));
	}

	closedir(d);

	return entries;
}

bool DirModule::Exists(const string& dir) const {
	return path.IsDir(dir);
}

// These create/remove a single or chained (all) directories.
// Note that Create fails on existing, but CreateAll does not.

bool DirModule::Create(const string& dir, const mode_t mode) const {
	string cdir(dir);
	if (mkdir(cdir.c_str(), mode) == -1) { // Default mode 0700
		perror("ERROR [DirModule] Failed to create directory");
		return false;
	}
	return true;
}

bool DirModule::CreateAll(const string& dir, const mode_t mode) const {
	const size_t len = dir.len();
	if (len == 0) {
		return false;
	}
	const unsigned char path_sep = path.PathSeparator;
	
	size_t s = 0;
	while (s + 1 < len) {
		if (dir.at(s) == path_sep && dir.at(s + 1) == path_sep) {
			s++;
			if (s + 1 == len) {
				return false; // Path of just /'s
			}
		} else {
			break;
		}
	}

	size_t x = s + (dir.at(s) == path_sep ? 1 : 0);
	while (x < len) {
		x = dir.indexOf(path_sep, x);

		// Check if exists
		const string& part = dir.sub(s, x);
		if (!Exists(part)) {
			if (!Create(part, mode)) {
				return false;
			}
		}

		if (x == string::NotFound || x >= dir.len() - 1) {
			break;
		} else {
			x++;
			while (x < len && dir.at(x) == path_sep) {
				x++;
			}
		}
	}
	return true;
}

bool DirModule::Remove(const string& dir) const {
	string cdir(dir);
	if (rmdir(cdir.c_str()) == -1) {
		perror("ERROR [DirModule]: Failed to remove directory");
		return false;
	}
	return true;
}

bool DirModule::RemoveAll(const string& dir) const {
	const size_t len = dir.len();
	if (len == 0) {
		return false;
	}
	const unsigned char path_sep = path.PathSeparator;
	size_t s = 0;
	while (s + 1 < len) {
		if (dir.at(s) == path_sep && dir.at(s + 1) == path_sep) {
			s++;
			if (s + 1 == len) {
				return false; // Path of just /'s
			}
		} else {
			break;
		}
	}
	size_t x = len;
	while (x > s) {
		while (dir.at(x - 1) == path_sep) {
			x--;
			if (x <= s) {
				return true; // Done
			}
		}

		// Check if exists
		const string& part = dir.sub(s, x);
		if (Exists(part)) {
			if (!Remove(part)) {
				return false;
			}
		}

		while (dir.at(x - 1) != path_sep) {
			x--;
			if (x <= s) {
				return true; // Done
			}
		}
	}
	return true;
}

// This deletes all directories and all files within. Careful!
bool DirModule::RemoveRecursive(const string& dir) const {
	array<string> list = List(dir);

	for (size_t i = 0; i < list.len(); i++) {
		const string& current = path.Join(dir, list.at(i));
		if (path.IsDir(current)) {
			if (!RemoveRecursive(current)) {
				return false;
			}
		} else { // Is a file
			if (!file.Delete(current)) {
				return false;
			}
		}
	}

	return Remove(dir);
}

} // namespace arc
