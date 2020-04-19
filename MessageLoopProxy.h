#pragma once
#include "lock.h"
#include "Callback.hpp"

class MessageLoop;
class MessageLoopProxy : public SupportWeakCallback
{
public:
	static std::shared_ptr<MessageLoopProxy> current();

	MessageLoopProxy();
	virtual ~MessageLoopProxy();

	void PostTask(const StdClosure &task);

private:
	mutable NLock message_loop_lock_;
	MessageLoop* target_message_loop_;
};

