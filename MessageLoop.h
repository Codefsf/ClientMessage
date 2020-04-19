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
	struct PendingTask
	{
		PendingTask(const StdClosure &task);
		~PendingTask();

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

	struct RunState
	{
		int run_depth;
		bool quit_received;
	};

	class AutoRunState : RunState
	{
	public:
		explicit AutoRunState(MessageLoop* loop);
		~AutoRunState();
	private:
		MessageLoop* loop_;
		RunState* previous_state_;
	};

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

	typedef std::priority_queue<PendingTask> DelayedTaskQueue;

	virtual bool DoWork();
	virtual bool DoIdleWork();

	void RunInternal();

	virtual void AddToIncomingQueue(const PendingTask &task);

	void ReloadWorkQueue();
	bool DeferOrRunPendingTask(const PendingTask &task);
	void RunTask(const PendingTask &task);
	bool DeletePendingTasks();

	std::shared_ptr<MessagePump> pump_;

	TaskQueue incoming_queue_;
	NLock incoming_queue_lock_;

	TaskQueue work_queue_;

	RunState *state_;

	std::shared_ptr<MessageLoopProxy> message_loop_proxy_;
};

