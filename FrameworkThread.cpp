#include "FrameworkThread.h"
#include "ThreadLocal.h"
#include "Thread.h"

ThreadLocalPointer<FrameworkThreadTlsData> *tlsData = new ThreadLocalPointer<FrameworkThreadTlsData>;

FrameworkThread::FrameworkThread(const char* name)
	: started_(false),
	stopping_(false),
	m_messageLoop(nullptr),
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
	if (m_messageLoop != nullptr)
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
	if (stopping_ || !m_messageLoop)
		return;

	stopping_ = true;
	m_messageLoop->PostTask(
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

		m_messageLoop = std::make_shared<MessageLoop>();

		Init();

		event_.Signal();

		m_messageLoop->Run();

		Cleanup();
		
		//m_messageLoop.reset(nullptr);
	}

	set_thread_id(invalidThreadId);
	{
		FrameworkThreadTlsData *tls = GetTlsData();
        if (tls != nullptr)
		{
		}
	}
	FreeTlsData();
}

void FrameworkThread::InitTlsData(FrameworkThread *self)
{
	FrameworkThreadTlsData *tls = GetTlsData();
    if (tls != nullptr)
		return;

	tls = new FrameworkThreadTlsData;
	tls->self = self;
	tls->managed = 0;
	tls->managed_thread_id = -1;
	tls->quit_properly = false;
    tls->custom_data = nullptr;
	tlsData->Set(tls);
}

void FrameworkThread::FreeTlsData()
{
	FrameworkThreadTlsData *tls = GetTlsData();
    if (tls == nullptr)
		return;

    tlsData->Set(nullptr);
	delete tls;
}

FrameworkThreadTlsData* FrameworkThread::GetTlsData()
{
	return tlsData->Get();
}

void FrameworkThread::SetThreadWasQuitProperly(bool flag)
{
	FrameworkThreadTlsData *tls = GetTlsData();
    if (tls == nullptr)
		return;

	tls->quit_properly = flag;
}

FrameworkThread* FrameworkThread::current()
{
	FrameworkThreadTlsData *tls = GetTlsData();
    if (tls == nullptr)
        return nullptr;

	return tls->self;
}

int FrameworkThread::GetManagedThreadId()
{
	FrameworkThreadTlsData *tls = GetTlsData();
    if (tls == nullptr)
		return -1;

	return tls->managed_thread_id;
}

void* FrameworkThread::GetCustomTlsData()
{
	FrameworkThreadTlsData *tls = GetTlsData();
    if (tls == nullptr)
        return nullptr;

	return tls->custom_data;
}

void FrameworkThread::SetCustomTlsData(void *data)
{
	FrameworkThreadTlsData *tls = GetTlsData();
    if (tls == nullptr)
		return;

	tls->custom_data = data;
}
