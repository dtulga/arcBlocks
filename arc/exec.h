#pragma once

#include "arc.h"
#include "string.h"

namespace arc {

// WARNING: Never use this module (especially shell-based functions) with user input! (Santized may be okay, depending upon syntax.) //

class ExecModule {
public:
	ExecModule() {}

	void SetAutoFail(bool active = true) { auto_fail_ = active; }
	void SetAutoFail(const char* message) { auto_fail_msg_ = message; auto_fail_ = true; }
	void SetAutoFail(const string& message) { auto_fail_msg_ = message; auto_fail_ = true; }

	void SetShell(const char* shell) { shell_ = shell; }
	void SetShell(const string& shell) { shell_ = shell; }

	//int Run(const char* command);
	int Run(const string& command) const;
	int RunShell(const string& command) const;

	//int Stream(const char* command);
	int Stream(const string& command) const;
	int StreamShell(const string& command) const;

	// These return: return code, stdout, stderr
	//multireturn3<int, string, string> RunGetOutput(const char* command) const;
	multireturn3<int, string, string> RunGetOutput(const string& command) const;

	// This sends all of the string input to stdin.
	multireturn3<int, string, string> RunGetOutputWithInput(const string& command, const string& input) const;

	// TODO: Stdin with stream/link

	// TODO: Run async with links for out, err, callback, wait, terminate/stop/pause, channels, promise/futures

private:
	bool auto_fail_ = false;
	string auto_fail_msg_;
	string shell_ = "/bin/sh"; // TODO: Windows/etc!

	DELETE_COPY_AND_ASSIGN(ExecModule);
} exec;

} // namespace arc
