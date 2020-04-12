#include "Thread.h"
#include <assert.h>

typedef struct tagTHREADNAME_INFO
{
	DWORD dwType; // Must be 0x1000.
	LPCSTR szName; // Pointer to name (in user addr space).
	DWORD dwThreadID; // Thread ID (-1=caller thread).
	DWORD dwFlags; // Reserved for future use, must be zero.
} THREADNAME_INFO;

Thread::Thread()
{
	thread_id_ = 0;
	thread_handle_ = NULL;
	thread_priority_ = kThreadPriorityDefault;
}

Thread::~Thread()
{
	Close();
}

bool Thread::Create()
{
	// create thread first
	thread_handle_ = (HANDLE)_beginthreadex(NULL,
		0, ThreadProcFunc, this, 0, (unsigned*)&thread_id_);

	if (thread_handle_ < (HANDLE)2)
	{
		thread_handle_ = NULL;
		thread_id_ = 0;
	}

	// if thread need set priority
	if (thread_priority_ > kThreadPriorityDefault)
	{
		// set the thread priority
		int pri = THREAD_PRIORITY_BELOW_NORMAL;
		if (thread_priority_ == kThreadPriorityLow)
			pri = THREAD_PRIORITY_LOWEST;
		else if (thread_priority_ == kThreadPriorityNormal)
			pri = THREAD_PRIORITY_BELOW_NORMAL;
		else if (thread_priority_ == kThreadPriorityHigh)
			pri = THREAD_PRIORITY_HIGHEST;
		else if (thread_priority_ == kThreadPriorityRealtime)
			pri = THREAD_PRIORITY_TIME_CRITICAL;
		SetThreadPriority(thread_handle_, pri);
	}

	return thread_handle_ > (HANDLE)1;
}

void Thread::Close()
{
	if (thread_handle_)
	{
		if (WAIT_OBJECT_0 == WaitForSingleObject(thread_handle_, INFINITE))
		{
			::CloseHandle(thread_handle_);
			thread_handle_ = NULL;
			thread_id_ = 0;
		}
		else
		{
			Terminate();
		}
	}
}

void Thread::Terminate()
{
	if (thread_handle_)
	{
		::TerminateThread(thread_handle_, 0);
		::CloseHandle(thread_handle_);
		thread_handle_ = NULL;
		thread_id_ = 0;
	}
}

void Thread::Sleep(int duration_ms)
{
	::Sleep(duration_ms);
}

void Thread::YieldThread()
{
	::Sleep(0);
}

ThreadId Thread::CurrentId()
{
	return GetCurrentThreadId();
}

uint32_t __stdcall Thread::ThreadProcFunc(void *arg)
{
	Thread* the_thread = (Thread*)arg;
	assert(the_thread);
	if (the_thread != NULL)
	{
		the_thread->Run();
	}
	_endthreadex(0);
	return 0;
}

