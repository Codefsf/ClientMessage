// A implemetation of a cross flatform waitable event based message loop 

#ifndef BASE_FRAMEWORK_DEFAULT_MESSAGE_PUMP_H_
#define BASE_FRAMEWORK_DEFAULT_MESSAGE_PUMP_H_

#include "MessagePump.h"
#include "waitable_event.h"
#include <QEventLoop>

class DefaultMessagePump : public MessagePump
{
public:

	DefaultMessagePump();
	virtual ~DefaultMessagePump() {}

	virtual void Run(Delegate* delegate);
	virtual void Quit();
	virtual void ScheduleWork();
	//virtual void ScheduleDelayedWork(const TimeTicks& delay_message_time);

private:
	void Wait();
	//void WaitTimeout(const TimeDelta &timeout);
	void Wakeup();

	WaitableEvent event_;
	bool should_quit_;
    QEventLoop m_loop;
	//TimeTicks delayed_work_time_;
};


#endif // BASE_FRAMEWORK_DEFAULT_MESSAGE_PUMP_H_
