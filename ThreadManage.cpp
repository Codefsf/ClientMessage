#include "ThreadManage.h"
#include "FrameworkThread.h"
#include "lock.h"

#define AUTO_MAP_LOCK() NAutoLock __l(&GetInstance()->lock_);
#define AQUIRE_ACCESS() {if (!AquireAccess()) { /*DCHECK(false);*/ return false; }}

bool ThreadMap::AquireAccess()
{
	FrameworkThreadTlsData *tls = FrameworkThread::GetTlsData();
	if (!tls || tls->managed < 1)
		return false;
	return true;
}

bool ThreadMap::RegisterThread(int self_identifier)
{
	//DCHECK(self_identifier >= 0);
	if (self_identifier < 0)
		return false;

	FrameworkThreadTlsData *tls = FrameworkThread::GetTlsData();
	//DCHECK(tls); // should be called by a Framework thread
	if (tls == NULL)
		return false;

	AUTO_MAP_LOCK()
		std::pair<std::map<int, FrameworkThread*>::iterator, bool> pr =
		ThreadMap::GetInstance()->threads_.insert(
			std::make_pair(self_identifier, tls->self));
	if (!pr.second)
	{
		if (pr.first->second != tls->self)
		{
			//DCHECK(false); // another thread has registered with the same id
			return false;
		}
		// yes, it's me, check logic error
		//DCHECK(tls->managed > 0);
		//DCHECK(tls->managed_thread_id == self_identifier);
	}
	// 'self' is registered
	tls->managed++;
	tls->managed_thread_id = self_identifier;
	return true;
}

bool ThreadMap::UnregisterThread()
{
	FrameworkThreadTlsData *tls = FrameworkThread::GetTlsData();
	//DCHECK(tls); // should be called by a Framework thread
	//DCHECK(tls->managed > 0); // should be managed
	if (!tls || tls->managed < 1)
		return false;

	// remove from internal thread map
	// here, since tls->managed is greater than zero,
	// we must have a reference of the glabal ThreadManager instance (see RegisterThread)
	if (--tls->managed == 0)
	{
		AUTO_MAP_LOCK()
			std::map<int, FrameworkThread*>::iterator iter = GetInstance()->threads_.find(tls->managed_thread_id);
		if (iter != GetInstance()->threads_.end())
			GetInstance()->threads_.erase(iter);
		else {}
		//DCHECK(false);	// logic error, we should not come here
		tls->managed_thread_id = -1;
	}

	return true;
}

FrameworkThread* ThreadMap::QueryThreadInternal(int identifier) const
{
	std::map<int, FrameworkThread*>::iterator iter
		= GetInstance()->threads_.find(identifier);
	if (iter == GetInstance()->threads_.end())
		return NULL;
	return iter->second;
}

int ThreadMap::QueryThreadId(const FrameworkThread *thread)
{
	AQUIRE_ACCESS()
		AUTO_MAP_LOCK()
		std::map<int, FrameworkThread*>::iterator iter;
	for (iter = GetInstance()->threads_.begin();
		iter != GetInstance()->threads_.end(); iter++) {
		if (iter->second == thread)
			return iter->first;
	}
	return -1;
}

std::shared_ptr<MessageLoopProxy> ThreadMap::GetMessageLoop(int identifier) const
{
	FrameworkThread *thread = QueryThreadInternal(identifier);
	if (thread == NULL)
		return NULL;
	MessageLoop *message_loop = thread->message_loop();
	if (message_loop == NULL)
		return NULL;
	return message_loop->message_loop_proxy();
}

//---------------------------------------------------------------------

bool ThreadManager::RegisterThread(int self_identifier)
{
	return ThreadMap::GetInstance()->RegisterThread(self_identifier);
}

bool ThreadManager::UnregisterThread()
{
	return ThreadMap::GetInstance()->UnregisterThread();
}

int ThreadManager::QueryThreadId(const FrameworkThread *thread)
{
	return ThreadMap::GetInstance()->QueryThreadId(thread);
}

FrameworkThread* ThreadManager::CurrentThread()
{
	FrameworkThreadTlsData *tls = FrameworkThread::GetTlsData();
	//DCHECK(tls); // should be called by a Framework thread
	//DCHECK(tls->managed > 0); // should be managed

	if (!tls || tls->managed < 1)
		return NULL;
	return tls->self;
}

bool ThreadManager::PostTask(const StdClosure &task)
{
	MessageLoop::current()->PostTask(task);
	return true;
}

bool ThreadManager::PostTask(int identifier, const StdClosure &task)
{
	std::shared_ptr<MessageLoopProxy> message_loop =
		ThreadMap::GetInstance()->GetMessageLoop(identifier);
	if (message_loop == NULL)
		return false;
	message_loop->PostTask(task);
	return true;
}
