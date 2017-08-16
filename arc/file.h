#pragma once

#include <exception>
#include <string>

#include "string.h"

namespace arc {

class FileModule {
public:
	FileModule() {}

	string GetContents(const string& filename, const bool binary = true) const;
	array<string> GetLines(const string& filename) const;

	size_t WriteContents(const string& filename, const string& data, const bool binary = true) const;
	size_t WriteLines(const string& filename, const array<string>& data) const;

	bool Exists(const string& filename) const;

	bool Delete(const string& filename) const;

	bool Rename(const string& old_filename, const string& new_filename) const;

	// TODO: GetContentsAsync, Stream, Append, Seek, File Class, TmpFile, etc.

private:
	DELETE_COPY_AND_ASSIGN(FileModule);
} file;

class file_error : public std::runtime_error {
public:
	explicit file_error(const std::string& what_arg) : std::runtime_error(what_arg) {}
	explicit file_error(const char* what_arg) : std::runtime_error(what_arg) {}
};

} // namespace arc
