#pragma once
#include "MessagePump.h"
#include <Windows.h>

typedef     signed __int64      int64_t;

class ProcessMessagePump : public MessagePump
{
public:

	//	UI��Ϣ�ɷ���
	//	UI��Ϣ�����ʹ����Ϣ�ɷ�������ô������ʹ�þ����
	//	TranslateMessage/DispatchMessageģʽ������Dispatcher�����
	class Dispatcher
	{
	public:

		virtual ~Dispatcher() {}
		virtual bool Dispatch(const MSG &message) = 0;
	};

	ProcessMessagePump() : have_work_(0), state_(NULL) {}
	virtual ~ProcessMessagePump() {}

	void RunWithDispatcher(Delegate* delegate, Dispatcher* dispatcher);

	virtual void Run(Delegate* delegate) { return RunWithDispatcher(delegate, NULL); }
	virtual void Quit();

protected:
	struct RunState
	{
		int run_depth;				// Ƕ�׵������
		bool should_quit;			// �Ƿ�Ӧ�������˳�
		Delegate* delegate;			// ���������ί��
		Dispatcher* dispatcher;		// ��Ϣ�ɷ���
	};

	// ȡ��ǰ��ʱ���
	int64_t GetCurrentDelay() const;
	virtual void DoRunLoop() = 0;

	// ��ʱ�����´����е�ʱ��
	//TimeTicks delayed_work_time_;

	// ָʾ��Ϣ�������Ƿ���kMsgDoWork��Ϣ
	long have_work_;

	// ָʾ��ǰMessagePump��״̬
	RunState* state_;
};

