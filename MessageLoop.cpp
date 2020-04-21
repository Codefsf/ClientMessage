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
	// ���������ܻ�����һ���߳��б�ִ�У����Ա����̰߳�ȫ
	std::shared_ptr<MessagePump> pump;
	{
		NAutoLock lock(&incoming_queue_lock_);
		bool was_empty = incoming_queue_.empty();
		incoming_queue_.push(task);
		if (!was_empty)
			return;
		// ��Ϊ�⺯�������Ǽ�ӵ�����һ���߳��б����õģ�
		// ��ʱMessageLoop�п����������������У�
		// ��Щ�����п��ܰ�������MessageLoop������
		// Ϊ�˱�֤��MessageLoop�е�MessagePump������Ч��
		// ������Ҫ�õ�����ָ��
		pump = pump_;
	}
	pump->ScheduleWork();
}

bool MessageLoop::DoWork()
{
	// ����ǰ�Ƿ�����ִ��
	//if (!nestable_tasks_allowed_)
	//	return false;

	for (;;)
	{
		// �ȴ�incoming����ȡ����
		ReloadWorkQueue();
		if (work_queue_.empty())
			break;

		// һ���Դ���work�����е���������
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
	// ����Idle״̬���ȳ���ִ�б������ŵķ�Ƕ������
	//if (ProcessNextDelayedNonNestableTask())
	//	return true;

	// ����˳����
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
		// ����ʱ�佻���ڴ�
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