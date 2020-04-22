// Copyright (c) 2011, NetEase Inc. All rights reserved.
//
// Author: wrt(guangguang)
// Date: 2011/6/24
//
// A implemetation of a cross flatform waitable event based message loop

#include "default_message_pump.h"
#include <cassert>
#include <QEventLoop>

DefaultMessagePump::DefaultMessagePump() :
	event_(false, false),
	should_quit_(0)
{

}

void DefaultMessagePump::Run(Delegate* delegate)
{
	assert(should_quit_ == false);

	for (;;)
	{
		bool did_work = delegate->DoWork();
		if (should_quit_)
			break;

		if (did_work)
			continue;

		did_work = delegate->DoIdleWork();
		if (should_quit_)
			break;

		if (did_work)
			continue;

        Wait();
	}

	should_quit_ = false;
}

void DefaultMessagePump::Quit()
{
	should_quit_ = true;
}

void DefaultMessagePump::ScheduleWork()
{
	// Since this can be called on any thread, we need to ensure that our Run
	// loop wakes up.
	Wakeup();
}

void DefaultMessagePump::Wait()
{
    //event_.Wait();
    m_loop.exec();
}

void DefaultMessagePump::Wakeup()
{
    //event_.Signal();
    m_loop.exit();
}

