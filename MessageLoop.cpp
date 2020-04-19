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
	if (!nestable_tasks_allowed_)
		return false;

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
			if (!task.delayed_run_time.is_null())
			{
				// 加入到定时任务队列
				AddToDelayedWorkQueue(task);
				// 如果加入的新任务是将被最先执行的，那么需要重新调度
				if (delayed_work_queue_.top().sequence_num == task.sequence_num)
					pump_->ScheduleDelayedWork(task.delayed_run_time);
			}
			else
			{
				if (DeferOrRunPendingTask(task))
					return true;
			}
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

bool MessageLoop::DeferOrRunPendingTask(const PendingTask &task)
{
	// 任务符合立即执行的条件，那么执行之
	if (task.nestable || state_->run_depth == 1)
	{
		RunTask(task);
		return true;
	}
	// 不可嵌套任务，需要缓存之直到在最顶层MessageLoop中执行
	deferred_non_nestable_work_queue_.push(task);
	return false;
}

void MessageLoop::RunTask(const PendingTask &task)
{
	assert(nestable_tasks_allowed_);

	// 考虑到最坏情况下，任务可能是不可重入的，
	// 所以暂时禁用嵌套任务

	nestable_tasks_allowed_ = false;
	PendingTask pending_task = task;
	PreProcessTask();
	pending_task.Run();
	PostPrecessTask();
	nestable_tasks_allowed_ = true;
}

bool MessageLoop::DeletePendingTasks()
{
	bool has_work = false;
	while (!work_queue_.empty())
	{
		PendingTask task = work_queue_.front();
		work_queue_.pop();
		if (!task.delayed_run_time.is_null())
			AddToDelayedWorkQueue(task);
	}

	while (!deferred_non_nestable_work_queue_.empty())
		deferred_non_nestable_work_queue_.pop();

	has_work = !delayed_work_queue_.empty();
	while (!delayed_work_queue_.empty())
		delayed_work_queue_.pop();

	return has_work;
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