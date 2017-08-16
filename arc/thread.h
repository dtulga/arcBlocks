#pragma once

#include <thread>
#include <atomic>
#include <chrono>
#include <vector>

#include "sync.h"

#define THREAD_STATE_NULL 0 /* Also Init */
#define THREAD_STATE_NATIVE 1
#define THREAD_STATE_RUNNING 2
#define THREAD_STATE_COMPLETE 3
#define THREAD_STATE_RET_FAILED 4
#define THREAD_STATE_EXCEPT_FAILED 5

namespace arc {

// Subclass this and override main() to perform work in another thread.
// Feel free to add any thread-local state too, but remember to use sync or link for inter-thread communication.
// Quick async functions can also use RunFunctionInThread but this disregards any return value, exceptions, or state.
class thread {
public:
	// All public functions are safe to call from outside of this thread.
	thread();
	thread(thread&& other);
	thread(std::thread&& native_thread); // Move from already-running thread, use this for RunFunctionInThread...

	thread& operator=(thread&& other);
	thread& operator=(std::thread&& native_thread);

	virtual ~thread() { wait(); }

	// Note that main_dispatch and main are run in the new thread.

	// Use this to start your thread asyncronously. Returns if the thread start (not finish) was successful.
	bool run();
	bool run_detach();

	void wait();
	void detach();

	std::thread::id id() noexcept { return thread_.get_id(); }
	bool running();
	uint8_t state() { return state_.load(); }
	uint8_t retCode() { return ret_code_.load(); } // Equals -1 when the thread has an exception, is not started/valid, or is native.

	// Or this if you also want to control the state changes (usually not recommended).
	virtual void main_dispatch();

protected:
	// Override this to do useful work.
	virtual int main() { return 0; }

	// These are safe to call within the running thread:
	std::thread::id thisThreadID() noexcept { return std::this_thread::get_id(); }

	void yield() noexcept { std::this_thread::yield(); }

	template <class Clock, class Duration>
	void sleepUntil(const std::chrono::time_point<Clock, Duration>& abs_time) { std::this_thread::sleep_until(abs_time); }

	template <class Rep, class Period>
	void sleepFor(const std::chrono::duration<Rep, Period>& rel_time) { std::this_thread::sleep_for(rel_time); }

	void sleep(const size_t seconds) { sleepFor(std::chrono::seconds(seconds)); }
	void sleepMilli(const size_t milliseconds) { sleepFor(std::chrono::milliseconds(milliseconds)); }
	void sleepMicro(const size_t microseconds) { sleepFor(std::chrono::microseconds(microseconds)); }
	void sleepNano(const size_t nanoseconds) { sleepFor(std::chrono::nanoseconds(nanoseconds)); }

	// Note that state above is also safe (although rarely useful) to call from within the running thread.

	std::thread thread_; // Actual running threads are moved into this.
	std::atomic<std::uint8_t> state_;
	std::atomic<std::uint8_t> ret_code_;

	DELETE_COPY_AND_ASSIGN(thread);
};

void ThreadDispatcher(thread* target);

// TODO! All thread objects stored in the ThreadModule, so memory management/lifetime is not a problem, the access as references.
// This also applies to RunFunctionInThread and CreateThreadFromObject, etc.

class ThreadManager	{
public:
	ThreadManager() {};
	~ThreadManager();

	thread& CreateThreadFromObject(thread* t_obj);
	thread& RunThreadFromObject(thread* t_obj);

	template< class Function, class... Args > 
	thread& RunFunctionInThread( Function&& f, Args&&... args ) {
		return CreateThreadFromObject( new thread(std::thread(std::move(f), std::move(args)...)) );
	}

	// May return 0 if unsupported!
	unsigned int HardwareConcurrency() { return hardware_concurrency_; }

	unsigned int RecommendedConcurrency() {
		return hardware_concurrency_ < 2 ? 2 : hardware_concurrency_;
	}

	// TODO: Thread pool, automatic management, etc.

private:
	std::vector<thread*> threads_;
	const unsigned int hardware_concurrency_ = std::thread::hardware_concurrency();

} thread_manager;

} // namespace arc
