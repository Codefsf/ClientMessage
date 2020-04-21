#pragma once
#include <memory>
#include <map>
#include "Callback.hpp"
#include "lock.h"

#define SINGLETON_DEFINE(TypeName)				\
static TypeName* GetInstance()					\
{												\
	static TypeName type_instance;				\
	return &type_instance;						\
}												

class FrameworkThread;
class MessageLoop;
class MessageLoopProxy;

class ThreadMap
{
public:
	SINGLETON_DEFINE(ThreadMap);
	static bool AquireAccess();

	bool RegisterThread(int self_identifier);
	bool UnregisterThread();
	int QueryThreadId(const FrameworkThread *thread);
	std::shared_ptr<MessageLoopProxy> GetMessageLoop(int identifier) const;
	FrameworkThread* QueryThreadInternal(int identifier) const;

private:
	ThreadMap() { }
	NLock lock_;
	std::map<int, FrameworkThread*> threads_;
};

class ThreadManager
{
public:
	static bool RegisterThread(int self_identifier);
	
	static bool UnregisterThread();

	static FrameworkThread* CurrentThread();
	template<typename T> static T* CurrentThreadT();
	static int QueryThreadId(const FrameworkThread *thread);

	static bool PostTask(const StdClosure &task);
	static bool PostTask(int identifier, const StdClosure &task);

	template<typename T1, typename T2>
	static bool Await(int identifier, const std::function<T1> &task, const std::function<T2> &reply)
	{
		std::shared_ptr<MessageLoopProxy> message_loop =
			ThreadMap::GetInstance()->GetMessageLoop(identifier);
		if (message_loop == NULL)
			return false;
		message_loop->PostTaskAndReply(task, reply);
		return true;
	}
};

