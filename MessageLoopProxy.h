#pragma once
#include "lock.h"
#include "Callback.hpp"

class MessageLoop;

class MessageLoopProxy : public SupportWeakCallback
{
public:
	friend class MessageLoop;
	friend struct MessageLoopProxyTraits;

	static std::shared_ptr<MessageLoopProxy> current();

	MessageLoopProxy();
	virtual ~MessageLoopProxy();
	virtual void OnDestruct() const;
	void WillDestroyCurrentMessageLoop();

	void PostTask(const StdClosure &task);

private:
	mutable NLock message_loop_lock_;
	MessageLoop* target_message_loop_;
};

struct MessageLoopProxyTraits
{
	static void Destruct(const MessageLoopProxy* proxy) {
		proxy->OnDestruct();
	}
};


