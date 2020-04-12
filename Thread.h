#pragma once
#include <process.h>
#include <Windows.h>
#include <stdint.h>

typedef uint32_t   ThreadId;
typedef void*      ThreadHandle;

const ThreadId invalidThreadId = 0;

enum ThreadPriority
{
	kThreadPriorityDefault,
	kThreadPriorityLow,
	kThreadPriorityNormal,
	kThreadPriorityHigh,
	kThreadPriorityRealtime
};

class Thread
{
public:
	Thread();
	virtual ~Thread();

	bool Create();

	void Close();

	void Terminate();

	static void Sleep(int duration_ms);

	static void YieldThread();

	static ThreadId CurrentId();

	ThreadId thread_id() const { return thread_id_; }

	void set_thread_id(ThreadId thread_id) { thread_id_ = thread_id; }

	ThreadHandle thread_handle() const { return thread_handle_; }

	void set_thread_priority(ThreadPriority priority) { thread_priority_ = priority; }

protected:
	virtual void Run() {}

private:
	static uint32_t __stdcall ThreadProcFunc(void *arg);

	ThreadId       thread_id_;
	ThreadHandle   thread_handle_;
	ThreadPriority thread_priority_;
};

