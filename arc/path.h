#pragma once

#include "string.h"

#ifdef _WIN32
	#define ARC_PATH_SEP_CHAR '\\'
	#define ARC_PATH_SEP_STR "\\"
	#define ARC_THIS_DIR_STR ".\\"
#else
	#define ARC_PATH_SEP_CHAR '/'
	#define ARC_PATH_SEP_STR "/"
	#define ARC_THIS_DIR_STR "./"
#endif

namespace arc {

class PathModule {
public:
	PathModule() {}

	// Gets: dir/dir/dir/file
	string File(const string& path);
	string Dir(const string& path);
	array<string> Split(const string& path); // Split on each path separator.

	// TODO:
	//string Root(); // C:\ or /
	//string RootOf(const string& path); // C:\, /, \\SHARE\NAME, nfs root, etc.

	bool IsAbsolute(const string& path);

	// Joins path fragments with OS-appropriate separators.
	string Join(const string& path1, const string& path2) const;
	string Join(const string& path1, const string& path2, const string& path3) const;
	string Join(const string& path1, const string& path2, const string& path3, const string& path4) const;
	// TODO: Any more than 4? Use a variadic function?

	bool CD(const char* path);
	bool CD(const string& path);

	const string& CurrentWorkingDir();
	const string& CurrentWorkingDirInternal() const { return working_dir_; }

	bool Exists(const string& path);
	bool IsDir(const string& path);
	bool IsFile(const string& path);

	static constexpr unsigned char const PathSeparator = ARC_PATH_SEP_CHAR;

private:
	void InternalJoin(string& joined, const string& to_join) const;

	string working_dir_;

	DELETE_COPY_AND_ASSIGN(PathModule);
} path;

} // namespace arc
