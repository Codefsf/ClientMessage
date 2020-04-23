#include "MessageLoop.h"
#include "ThreadLocal.h"

#include <assert.h>
#include <thread>
#include <functional>
#include <iostream>

using namespace std;

PendingTask::PendingTask(const StdClosure &task)
    : std_task(task)
{
}

PendingTask::~PendingTask()
{

}

ThreadLocalPointer<MessageLoop> g_lazy_ptr;

MessageLoop::MessageLoop() :
    QObject(nullptr),
	state_(nullptr)
{
    m_eventLoop.reset(new QEventLoop);

	g_lazy_ptr.Set(this);

    connect(this, &MessageLoop::scheduleWork, this, &MessageLoop::doWork);
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

    g_lazy_ptr.Set(nullptr);
}

MessageLoop* MessageLoop::current()
{
	return g_lazy_ptr.Get();
}

void MessageLoop::Run()
{
	assert(this == current());
    //AutoRunState state(this);
	RunInternal();
}

void MessageLoop::RunInternal()
{
	assert(this == current());

    m_eventLoop->exec();
    //pump_->Run(this);
}

void MessageLoop::Quit()
{
	if (state_)
		state_->quit_received = true;
}

void MessageLoop::QuitNow()
{
    m_eventLoop->exit();
    //pump_->Quit();
}

void MessageLoop::PostTask(const StdClosure &task)
{
	PendingTask pending_task(task);
	AddToIncomingQueue(pending_task);
}

void MessageLoop::AddToIncomingQueue(const PendingTask &task)
{
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
        //pump = pump_;
	}

    emit scheduleWork();
}

void MessageLoop::doWork()
{
    DoWork();
}

bool MessageLoop::DoWork()
{
    while (true)
	{
		ReloadWorkQueue();
		if (work_queue_.empty())
			break;

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
	while (!work_queue_.empty())
	{
		PendingTask task = work_queue_.front();
		work_queue_.pop();
	}

    return true;
}

