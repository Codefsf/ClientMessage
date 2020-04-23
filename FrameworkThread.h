#pragma once
#include "Callback.hpp"
#include "Thread.h"
#include "MessageLoop.h"
#include "waitable_event.h"

#include <memory>

class FrameworkThread;
class Dispatcher;

struct FrameworkThreadTlsData
{
	FrameworkThread *self;		// A pointer pointed to the thread itself
	bool quit_properly;			// the thread quit properly
	int managed;				// reference counter, the framework thread is managed by ThreadManager
	int managed_thread_id;		// if a thread is managered, this will be the manager id of it (may not be a thread id)
	void *custom_data;			// reserved for the derived classes of FrameworkThread
};

class FrameworkThread : public virtual SupportWeakCallback, public Thread
{
	friend class ThreadMap;
	friend class ThreadManager;
public:
	// custom message loop
	class CustomMessageLoop : public MessageLoop
	{
	public:
		virtual ~CustomMessageLoop() {}
	};

	class CustomMessageLoopFactory
	{
	public:
		virtual CustomMessageLoop* CreateMessageLoop() = 0;
	};

	explicit FrameworkThread(const char* name);

	virtual ~FrameworkThread();

	void RunWithLoop();

	bool Start();

	bool StartWithLoop();

	void Stop();

	void StopSoon();

	std::shared_ptr<MessageLoop> message_loop() const { return m_messageLoop; }

	const std::string &thread_name() { return name_; }

	bool IsRunning() const { return (Thread::thread_id() != invalidThreadId) ? true : false; }

	bool IsStopping() const { return stopping_; }

	static FrameworkThread* current();

	static int GetManagedThreadId();

	static void* GetCustomTlsData();

	static void SetCustomTlsData(void *data);

protected:
	virtual void Init() {}

	virtual void Run();

	virtual void Cleanup() {}

	static void InitTlsData(FrameworkThread *self);

	static void FreeTlsData();

	static FrameworkThreadTlsData* GetTlsData();
	
	static void SetThreadWasQuitProperly(bool flag);

	void set_message_loop(MessageLoop* message_loop)
	{	
		if (message_loop)
			m_messageLoop.reset(message_loop);
	}

private:
	bool thread_was_started() const { return started_; }

	void DoStopSoon();

	bool started_;

	bool stopping_;

	std::shared_ptr<MessageLoop> m_messageLoop;
	std::shared_ptr<CustomMessageLoopFactory> factory_;

	WaitableEvent event_;

	std::string name_;
};

