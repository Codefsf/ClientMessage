#pragma once

#include <queue>								
#include <memory>
#include "Callback.hpp"

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

	// MessageLoop类型
	Type type_;
	// 运行状态
	RunState *state_;
	// 消息泵
	std::shared_ptr<MessagePump> pump_;

	// 任务输入队列，任何经过Post族函数加入的任务都首先进入该队列，之后由运行Run的线程分配到各个专职队列
	TaskQueue incoming_queue_;
	// 输入队列锁
	NLock incoming_queue_lock_;

	// 这个工作队列的设计是为了最大限度减少对输入队列的加锁操作。
	// 这种关系类似于双缓存设计，输入队列暂存本线程和其他线程投递过来的任务，
	// 工作队列则仅仅被运行Run方法的线程操作，只有线程检查到工作队列为空才会去输入队列拉任务放到工作队列然后逐个运行。
	// 这里采用了交换内存的方法来提高拉任务的效率，也就是说拉任务的操作其实不是一个一个从输入队列中移动任务来完成的，
	// 而是直接交换双方的内存，这样交换后输入队列瞬时变空，而任务队列则拥有了之前输入队列所有的数据。
	TaskQueue work_queue_;

	
	// The message loop proxy associated with this message loop, if one exists.
	std::shared_ptr<MessageLoopProxy> message_loop_proxy_;
	};
};

