#include "thread.h"

#include "log.h"

namespace arc {

void ThreadDispatcher(thread* target) {
	if (target != nullptr) {
		target->main_dispatch();
	}
}

thread::thread() {
	state_.store(THREAD_STATE_NULL);
	ret_code_.store((uint8_t) -1);
}

thread::thread(thread&& other) : thread_(std::move(other.thread_)) {
	state_.store(other.state_.load());
	ret_code_.store(other.ret_code_.load());
}

thread::thread(std::thread&& native_thread) : thread_(std::move(native_thread)) { // Move from already-running thread, use this for RunFunctionInThread
	state_.store(THREAD_STATE_NATIVE);
	ret_code_.store((uint8_t) -1);
}

thread& thread::operator=(thread&& other) {
	wait(); // To remove any already-existing threads running here.
	thread_ = std::move(other.thread_);
	state_.store(other.state_.load());
	ret_code_.store(other.ret_code_.load());
	return *this;
}

thread& thread::operator=(std::thread&& native_thread) { // Move from already-running thread, use this for RunFunctionInThread
	wait(); // To remove any already-existing threads running here.
	thread_ = std::move(native_thread);
	state_.store(THREAD_STATE_NATIVE);
	ret_code_.store((uint8_t) -1);
	return *this;
}

// Use this to start your thread asyncronously.
bool thread::run() {
	if (running()) {
		return false;
	}
	
	thread_ = std::thread(ThreadDispatcher, this);
	return thread_.joinable();
}

bool thread::run_detach() {
	if (running()) {
		return false;
	}
	
	thread_ = std::thread(ThreadDispatcher, this);
	
	if (thread_.joinable()) {
		thread_.detach();
		return true;
	}
	return false;
}

void thread::wait() {
	if (thread_.joinable()) {
		thread_.join();
	}
}

void thread::detach() {
	if (thread_.joinable()) {
		thread_.detach();
	}
	state_.store(THREAD_STATE_NULL);
	ret_code_.store((uint8_t) -1);
}

bool thread::running() {
	const uint8_t st = state_.load();
	switch (st) {
		case THREAD_STATE_COMPLETE:
		case THREAD_STATE_RET_FAILED:
		case THREAD_STATE_EXCEPT_FAILED:
		return false;
		default:
		return thread_.joinable();
	}
}

void thread::main_dispatch() {
	state_.store(THREAD_STATE_RUNNING);
	try {
		ret_code_ = main();
	} catch (std::exception e) {
		log::Error("Exception encountered in thread " + string::itoa(std::hash<std::thread::id>()(thread_.get_id())) + ": " + string(e.what()));
		state_.store(THREAD_STATE_EXCEPT_FAILED);
		return;
	}
	state_.store(ret_code_ == 0 ? THREAD_STATE_COMPLETE : THREAD_STATE_RET_FAILED);
}

ThreadManager::~ThreadManager() {
	const size_t len = threads_.size();
	for (size_t i = 0; i < len; i++) {
		if (threads_[i] != nullptr) {
			threads_[i]->wait();
		}
	}

	for (size_t i = 0; i < len; i++) {
		if (threads_[i] != nullptr) {
			delete threads_[i];
			threads_[i] = nullptr;
		}
	}

	threads_.clear();
}

thread& ThreadManager::CreateThreadFromObject(thread* t_obj) {
	const size_t len = threads_.size();
	for (size_t i = 0; i < len; i++) {
		if (threads_[i] == nullptr) {
			threads_[i] = t_obj;
			return *(threads_[i]);
		}
	}
	threads_.push_back(t_obj);
	return *(threads_[len]);
}

thread& ThreadManager::RunThreadFromObject(thread* t_obj) {
	thread& th = CreateThreadFromObject(t_obj);
	th.run();
	return th;
}

} // namespace arc
