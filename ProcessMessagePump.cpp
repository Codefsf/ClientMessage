#include "ProcessMessagePump.h"

#include <assert.h>
#include <math.h>

void ProcessMessagePump::RunWithDispatcher(Delegate* delegate, Dispatcher* dispatcher)
{
	assert(delegate);

	RunState rs;
	rs.delegate = delegate;
	rs.dispatcher = dispatcher;
	rs.should_quit = false;
	rs.run_depth = state_ ? state_->run_depth + 1 : 1;

	RunState* previous_state = state_;
	state_ = &rs;

	DoRunLoop();

	state_ = previous_state;
}

void ProcessMessagePump::Quit()
{
	assert(state_);
	if (state_)
		state_->should_quit = true;
}

int64_t ProcessMessagePump::GetCurrentDelay() const
{
	if (delayed_work_time_.is_null())
		return -1;

	// ��΢��ľ���ת��Ϊ����
	double timeout = ceil((delayed_work_time_ - TimeTicks::Now()).ToInternalValue() / 1000.0);

	// ���delay��ֵ�Ǹ��ģ���ô��ʾ������Ҫ��Խ������Խ��
	int64_t delay = static_cast<int64_t>(timeout);
	if (delay < 0)
		delay = 0;

	return delay;
}
