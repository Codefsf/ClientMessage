#pragma once
#include <memory>
#include <map>
#include "Callback.hpp"

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
	//NLock lock_;
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

	static bool PostDelayedTask(const StdClosure &task, TimeDelta delay);
	static bool PostDelayedTask(int identifier, const StdClosure &task, TimeDelta delay);

	static const int TIMES_FOREVER = -1;
	static void PostRepeatedTask(const WeakCallback<StdClosure>& task, const TimeDelta& delay, int times = TIMES_FOREVER);
	static void PostRepeatedTask(int thread_id, const WeakCallback<StdClosure>& task, const TimeDelta& delay, int times = TIMES_FOREVER);

	static bool PostNonNestableTask(const StdClosure &task);
	static bool PostNonNestableTask(int identifier, const StdClosure &task);

	static bool PostNonNestableDelayedTask(const StdClosure &task, TimeDelta delay);
	static bool PostNonNestableDelayedTask(int identifier, const StdClosure &task, TimeDelta delay);

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

private:
	static void RunRepeatedly(const WeakCallback<StdClosure>& task, const TimeDelta& delay, int times);
	static void RunRepeatedly2(int thread_id, const WeakCallback<StdClosure>& task, const TimeDelta& delay, int times);
};
