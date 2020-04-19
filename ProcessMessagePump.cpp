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