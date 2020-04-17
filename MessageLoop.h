#pragma once

#include <queue>								
#include <memory>
#include <assert.h>

#include "Callback.hpp"
#include "MessageLoopProxy.h"

class MessagePump;

class MessageLoop
{
public:
	explicit MessageLoop();
	virtual ~MessageLoop();
	static MessageLoop* current();

	MessagePump * pump() { return pump_.get(); }

	void Run();

	void Quit();

	void QuitNow();

	void PostTask(const StdClosure &task);

	std::shared_ptr<MessageLoopProxy> message_loop_proxy() {
		return message_loop_proxy_;
	}

	struct PendingTask
	{
		PendingTask(const StdClosure &task);
		~PendingTask();

		bool operator<(const PendingTask& other) const;

		bool nestable;

		void Run()
		{
			if (std_task) {
				std_task();
			}
			else {
				assert(false);
			}
		}

	private:
		StdClosure std_task;
	};

	class TaskQueue : public std::queue<PendingTask>
	{
	public:
		void Swap(TaskQueue* queue)
		{
			c.swap(queue->c);  // 常数时间复杂度的内存交换
		}
	};

	typedef std::priority_queue<PendingTask> DelayedTaskQueue;

	virtual bool DoWork();
	virtual bool DoIdleWork();

	void RunInternal();

	// AddToIncomingQueue函数线程安全，其余均为不线程安全
	virtual void AddToIncomingQueue(const PendingTask &task);
	void ReloadWorkQueue();
	bool DeferOrRunPendingTask(const PendingTask &task);
	void RunTask(const PendingTask &task);
	bool DeletePendingTasks();

	//RunState *state_;

	std::shared_ptr<MessagePump> pump_;

	TaskQueue incoming_queue_;
	NLock incoming_queue_lock_;

	TaskQueue work_queue_;

	
	// The message loop proxy associated with this message loop, if one exists.
	std::shared_ptr<MessageLoopProxy> message_loop_proxy_;
	};
};

