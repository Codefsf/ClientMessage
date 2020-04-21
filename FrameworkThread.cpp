#include "FrameworkThread.h"
#include "ThreadLocal.h"
#include "Thread.h"

ThreadLocalPointer<FrameworkThreadTlsData> *tlsData = new ThreadLocalPointer<FrameworkThreadTlsData>;

FrameworkThread::FrameworkThread(const char* name)
	: started_(false),
	stopping_(false),
	message_loop_(NULL),
	event_(false, false),
	name_(name)
{

}

FrameworkThread::~FrameworkThread()
{
	Stop();
}

void FrameworkThread::RunWithLoop() {
	Run();
}

bool FrameworkThread::Start()
{
	return StartWithLoop();
}

bool FrameworkThread::StartWithLoop()
{
	if (message_loop_ != NULL)
		return false;

	if (Create() == false)
		return false;

	event_.Wait();

	started_ = true;
	return true;
}

void FrameworkThread::Stop()
{
	if (!thread_was_started())
		return;

	StopSoon();

	Close();

	started_ = false;
	stopping_ = false;
}

void FrameworkThread::StopSoon()
{
	if (stopping_ || !message_loop_)
		return;

	stopping_ = true;
	message_loop_->PostTask(
		Bind(&FrameworkThread::DoStopSoon, this));
}

void FrameworkThread::DoStopSoon()
{
	MessageLoop::current()->Quit();
	SetThreadWasQuitProperly(true);
}

void FrameworkThread::Run()
{
	InitTlsData(this);
	//SetThreadWasQuitProperly(false);
	{
		ThreadId thread_id = Thread::CurrentId();
		set_thread_id(thread_id);

		message_loop_ = new MessageLoop;

		// Let the thread do extra initialization.
		// Let's do this before signaling we are started.
		Init();

		event_.Signal();

		message_loop_->Run();

		Cleanup();

		delete message_loop_;
		
		message_loop_ = nullptr;
	}

	set_thread_id(invalidThreadId);
	{
		FrameworkThreadTlsData *tls = GetTlsData();
		if (tls != NULL)
		{
		}
	}
	FreeTlsData();
}

void FrameworkThread::InitTlsData(FrameworkThread *self)
{
	FrameworkThreadTlsData *tls = GetTlsData();
	//DCHECK(tls == NULL);
	if (tls != NULL)
		return;
	tls = new FrameworkThreadTlsData;
	tls->self = self;
	tls->managed = 0;
	tls->managed_thread_id = -1;
	tls->quit_properly = false;
	tls->custom_data = NULL;
	tlsData->Set(tls);
}

void FrameworkThread::FreeTlsData()
{
	FrameworkThreadTlsData *tls = GetTlsData();
	if (tls == NULL)
		return;
	tlsData->Set(NULL);
	delete tls;
}

FrameworkThreadTlsData* FrameworkThread::GetTlsData()
{
	return tlsData->Get();
}

void FrameworkThread::SetThreadWasQuitProperly(bool flag)
{
	FrameworkThreadTlsData *tls = GetTlsData();
	if (tls == NULL)
		return;
	tls->quit_properly = flag;
}

FrameworkThread* FrameworkThread::current()
{
	FrameworkThreadTlsData *tls = GetTlsData();
	//DCHECK(tls != NULL);
	if (tls == NULL)
		return NULL;
	return tls->self;
}

int FrameworkThread::GetManagedThreadId()
{
	FrameworkThreadTlsData *tls = GetTlsData();
	//DCHECK(tls != NULL);
	if (tls == NULL)
		return -1;
	return tls->managed_thread_id;
}

void* FrameworkThread::GetCustomTlsData()
{
	FrameworkThreadTlsData *tls = GetTlsData();
	//DCHECK(tls != NULL);
	if (tls == NULL)
		return NULL;
	return tls->custom_data;
}

void FrameworkThread::SetCustomTlsData(void *data)
{
	FrameworkThreadTlsData *tls = GetTlsData();
	//DCHECK(tls != NULL);
	if (tls == NULL)
		return;
	tls->custom_data = data;
}