#include <memory>

#include "MessageLoopProxy.h"
#include "Callback.hpp"
#include "MessageLoop.h"

std::shared_ptr<MessageLoopProxy> MessageLoopProxy::current()
{
	MessageLoop* cur_loop = MessageLoop::current();
	if (!cur_loop)
		return nullptr;
	return cur_loop->message_loop_proxy();
}

MessageLoopProxy::MessageLoopProxy() :
	target_message_loop_(MessageLoop::current())
{

}

MessageLoopProxy::~MessageLoopProxy()
{

}

void MessageLoopProxy::OnDestruct() const
{
	delete this;
}

void MessageLoopProxy::PostTask(const StdClosure &task) 
{
	NAutoLock lock(&message_loop_lock_);

	target_message_loop_->PostTask(task);
}

void MessageLoopProxy::WillDestroyCurrentMessageLoop()
{
	NAutoLock lock(&message_loop_lock_);
	target_message_loop_ = nullptr;
}