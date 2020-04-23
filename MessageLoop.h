#pragma once

#include <queue>								
#include <memory>
#include <assert.h>

#include "lock.h"
#include "Callback.hpp"
#include "MessagePump.h"

#include <QEventLoop>
#include <QObject>

class MessagePump;

struct PendingTask
{
    PendingTask(const StdClosure &task);
    ~PendingTask();

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
        c.swap(queue->c);  // 常数时间复杂度的内存交换
    }
};

struct RunState
{
    int run_depth;
    bool quit_received;
};

//class AutoRunState : RunState
//{
//public:
//    explicit AutoRunState(MessageLoop* loop);
//    ~AutoRunState();
//private:
//    MessageLoop* loop_;
//    RunState* previous_state_;
//};

class MessageLoop : public QObject
{
    Q_OBJECT

public:

	explicit MessageLoop();
	virtual ~MessageLoop();
	static MessageLoop* current();

	MessagePump * pump() { return pump_.get(); }

	void Run();

	void Quit();

	void QuitNow();

	void PostTask(const StdClosure &task);

    typedef std::priority_queue<PendingTask> DelayedTaskQueue;

	virtual bool DoWork();
	virtual bool DoIdleWork();

	void RunInternal();

	virtual void AddToIncomingQueue(const PendingTask &task);

	void ReloadWorkQueue();
	void RunTask(const PendingTask &task);
	bool DeletePendingTasks();

signals:
    void scheduleWork();

public slots:
    void doWork();

private:
    std::unique_ptr<QEventLoop> m_eventLoop;

	std::shared_ptr<MessagePump> pump_;

	TaskQueue incoming_queue_;
	NLock incoming_queue_lock_;

	TaskQueue work_queue_;

	RunState *state_;
};

