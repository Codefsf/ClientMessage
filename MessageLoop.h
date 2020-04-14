#pragma once

#include <queue>								
#include <memory>
#include "Callback.hpp"

class MessagePump;

class MessageLoop
{
public:
	explicit MessageLoop();
	virtual ~MessageLoop();
	static MessageLoop* current();

	MessagePump * pump() { return pump_.get(); }

	void Run();

	void Quit();

	void QuitNow();

	void PostTask(const StdClosure &task);

	std::shared_ptr<MessageLoopProxy> message_loop_proxy() {
		return message_loop_proxy_;
	}

	struct PendingTask
	{
		PendingTask(const StdClosure &task);
		~PendingTask();

		bool operator<(const PendingTask& other) const;

		bool nestable;

		void Run()
		{
			if (std_task) {
				std_task();
			}
			else {
				assert(false);
			}
		}

	private:
		StdClosure std_task;
	};

	class TaskQueue : public std::queue<PendingTask>
	{
	public:
		void Swap(TaskQueue* queue)
		{
			c.swap(queue->c);  // ����ʱ�临�Ӷȵ��ڴ潻��
		}
	};

	typedef std::priority_queue<PendingTask> DelayedTaskQueue;

	virtual bool DoWork();
	virtual bool DoIdleWork();

	void RunInternal();

	// AddToIncomingQueue�����̰߳�ȫ�������Ϊ���̰߳�ȫ
	virtual void AddToIncomingQueue(const PendingTask &task);
	void ReloadWorkQueue();
	bool DeferOrRunPendingTask(const PendingTask &task);
	void RunTask(const PendingTask &task);
	bool DeletePendingTasks();

	// MessageLoop����
	Type type_;
	// ����״̬
	RunState *state_;
	// ��Ϣ��
	std::shared_ptr<MessagePump> pump_;

	// ����������У��κξ���Post�庯��������������Ƚ���ö��У�֮��������Run���̷߳��䵽����רְ����
	TaskQueue incoming_queue_;
	// ���������
	NLock incoming_queue_lock_;

	// ����������е������Ϊ������޶ȼ��ٶ�������еļ���������
	// ���ֹ�ϵ������˫������ƣ���������ݴ汾�̺߳������߳�Ͷ�ݹ���������
	// �������������������Run�������̲߳�����ֻ���̼߳�鵽��������Ϊ�ղŻ�ȥ�������������ŵ���������Ȼ��������С�
	// ��������˽����ڴ�ķ���������������Ч�ʣ�Ҳ����˵������Ĳ�����ʵ����һ��һ��������������ƶ���������ɵģ�
	// ����ֱ�ӽ���˫�����ڴ棬�����������������˲ʱ��գ������������ӵ����֮ǰ����������е����ݡ�
	TaskQueue work_queue_;

	
	// The message loop proxy associated with this message loop, if one exists.
	std::shared_ptr<MessageLoopProxy> message_loop_proxy_;
	};
};

