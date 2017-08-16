#include "file.h"

#include "path.h"

#include <stdio.h>

namespace arc {

string FileModule::GetContents(const string& filename, const bool binary) const {
	string file_path = filename;
	
	if (!path.IsAbsolute(file_path)) {
		const string& working_dir = path.CurrentWorkingDirInternal();
		if (working_dir.len() > 0) {
			file_path = path.Join(working_dir, file_path);
		}
	}

	string file_contents;

	FILE* f = fopen(file_path.c_str(), binary ? "rb" : "r");
	if (f == nullptr) {
		perror("Error [FileModule.GetContents]: file open failed");
		throw file_error("file open failed");
	}

	fseek(f, 0, SEEK_END); // seek to end of file
	auto ssize = ftell(f); // get current file pointer
	if (ssize <= 0) {
		fclose(f);
		return file_contents;
	}
	rewind(f);
	size_t size = ssize;

	file_contents.reserve(size);

	if (fread(file_contents.mutable_data(), 1, size, f) != size) {
		perror("Error [FileModule.GetContents]: file read failed");
		fclose(f);
		throw file_error("file read failed");
	}
	
    fclose(f);
    return file_contents;
}

array<string> FileModule::GetLines(const string& filename) const {
	return GetContents(filename, false).split(string("\n"));
}

size_t FileModule::WriteContents(const string& filename, const string& data, const bool binary) const {
	string file_path = filename;
	
	if (!path.IsAbsolute(file_path)) {
		const string& working_dir = path.CurrentWorkingDirInternal();
		if (working_dir.len() > 0) {
			file_path = path.Join(working_dir, file_path);
		}
	}

	FILE* f = fopen(file_path.c_str(), binary ? "wb" : "w"); // Note that this already truncates it to zero length.
	if (f == nullptr) {
		perror("Error [FileModule.WriteContents]: file open for write failed");
		throw file_error("file open failed");
	}

	const unsigned char* bytes = data.data();
	const size_t len = data.len();
	size_t written = fwrite(bytes, 1, len, f);

	if (written != len) {
		perror("Error [FileModule.WriteContents]: file write failed");
		fclose(f);
		throw file_error("file write failed");
	}

	fclose(f);

	return written;
}

size_t FileModule::WriteLines(const string& filename, const array<string>& data) const {
	return WriteContents(filename, data.join(string("\n")), false);
}

bool FileModule::Exists(const string& filename) const {
	return path.IsFile(filename);
}

bool FileModule::Delete(const string& filename) const {
	string cname(filename);
	if (remove(cname.c_str()) != 0) {
		perror("Error [FileModule]: file delete failed");
		return false;
	}
	return true;
}

bool FileModule::Rename(const string& old_filename, const string& new_filename) const {
	string old_cname(old_filename);
	string new_cname(new_filename);
	if (rename(old_cname.c_str(), new_cname.c_str()) != 0) {
		perror("Error [FileModule]: file rename failed");
		return false;
	}
	return true;
}

} // namespace arc
