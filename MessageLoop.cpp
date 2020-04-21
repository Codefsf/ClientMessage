#include "MessageLoop.h"
#include "ThreadLocal.h"
#include "MessageLoopProxy.h"
#include "default_message_pump.h"

#include <assert.h>

MessageLoop::PendingTask::PendingTask(const StdClosure &task)
	: std_task(task)
{

}

MessageLoop::PendingTask::~PendingTask()
{

}

ThreadLocalPointer<MessageLoop> g_lazy_ptr;

MessageLoop::MessageLoop() :
	state_(nullptr)
{
	pump_.reset(new DefaultMessagePump);
	g_lazy_ptr.Set(this);

	message_loop_proxy_.reset(new MessageLoopProxy, &MessageLoopProxyTraits::Destruct);
	message_loop_proxy_->target_message_loop_ = this;
}

MessageLoop::~MessageLoop()
{
	bool has_work = false;

	for (int i = 0; i < 100; i++)
	{
		DeletePendingTasks();
		ReloadWorkQueue();
		has_work = DeletePendingTasks();
		if (!has_work)
			break;
	}

	assert(!has_work);

	message_loop_proxy_->WillDestroyCurrentMessageLoop();
	message_loop_proxy_ = nullptr;

	g_lazy_ptr.Set(NULL);
}

MessageLoop* MessageLoop::current()
{
	return g_lazy_ptr.Get();
}

void MessageLoop::Run()
{
	assert(this == current());
	AutoRunState state(this);
	RunInternal();
}

void MessageLoop::RunInternal()
{
	assert(this == current());

	pump_->Run(this);
}

void MessageLoop::Quit()
{
	if (state_)
		state_->quit_received = true;
}

void MessageLoop::QuitNow()
{
	if (pump_)
		pump_->Quit();
}

void MessageLoop::PostTask(const StdClosure &task)
{
	PendingTask pending_task(task);
	AddToIncomingQueue(pending_task);
}

void MessageLoop::AddToIncomingQueue(const PendingTask &task)
{
	// 本方法可能会在另一个线程中被执行，所以必须线程安全
	std::shared_ptr<MessagePump> pump;
	{
		NAutoLock lock(&incoming_queue_lock_);
		bool was_empty = incoming_queue_.empty();
		incoming_queue_.push(task);
		if (!was_empty)
			return;
		// 因为这函数可能是间接地在另一个线程中被调用的，
		// 此时MessageLoop中可能正有任务在运行，
		// 这些任务中可能包含销毁MessageLoop的任务，
		// 为了保证对MessageLoop中的MessagePump引用有效，
		// 这里需要用到引用指针
		pump = pump_;
	}
	pump->ScheduleWork();
}

bool MessageLoop::DoWork()
{
	// 任务当前是否允许被执行
	//if (!nestable_tasks_allowed_)
	//	return false;

	for (;;)
	{
		// 先从incoming队列取任务
		ReloadWorkQueue();
		if (work_queue_.empty())
			break;

		// 一次性处理work队列中的所有任务
		do
		{
			PendingTask task = work_queue_.front();
			work_queue_.pop();

			RunTask(task);
		} while (!work_queue_.empty());
	}

	return false;
}

bool MessageLoop::DoIdleWork()
{
	// 进入Idle状态后，先尝试执行被缓存着的非嵌套任务
	//if (ProcessNextDelayedNonNestableTask())
	//	return true;

	// 检查退出标记
	if (state_->quit_received)
		pump_->Quit();

	return false;
}

void MessageLoop::ReloadWorkQueue()
{
	if (!work_queue_.empty())
		return;

	{
		NAutoLock lock(&incoming_queue_lock_);
		if (incoming_queue_.empty())
			return;
		// 常数时间交换内存
		work_queue_.Swap(&incoming_queue_);
	}
}

void MessageLoop::RunTask(const PendingTask &task)
{
	PendingTask pending_task = task;
	pending_task.Run();
}

bool MessageLoop::DeletePendingTasks()
{
	bool has_work = false;
	while (!work_queue_.empty())
	{
		PendingTask task = work_queue_.front();
		work_queue_.pop();
	}

	return true;
}

//----------------------------------------------------------------

MessageLoop::AutoRunState::AutoRunState(MessageLoop* loop) :
	loop_(loop)
{
	previous_state_ = loop_->state_;
	if (previous_state_) {
		run_depth = previous_state_->run_depth + 1;
	}
	else {
		run_depth = 1;
	}
	loop_->state_ = this;

	// Initialize the other fields:
	quit_received = false;
}

MessageLoop::AutoRunState::~AutoRunState()
{
	loop_->state_ = previous_state_;
}