#pragma once

#include "string.h"

namespace arc {

class DirModule {
public:
	DirModule() {}

	array<string> List() const; // Lists the contents of the current working dir.
	array<string> List(const string& dir) const; // Lists the contents of the given dir.

	bool Exists(const string& dir) const;

	// These create/remove a single or chained (all) directories.
	bool Create(const string& dir, const mode_t mode = 0700) const;
	bool CreateAll(const string& dir, const mode_t mode = 0700) const;
	bool Remove(const string& dir) const;
	bool RemoveAll(const string& dir) const;
	// This deletes all directories and all files within. Careful!
	bool RemoveRecursive(const string& dir) const;

private:
	DELETE_COPY_AND_ASSIGN(DirModule);
} dir;

} // namespace arc
