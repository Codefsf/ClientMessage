#pragma once

// MessagePump���������������͵���Ϣѭ��������UI��Ϣ��IO��Ϣ�ȣ���˿�������ΪUIMessagePump��IOMessagePump�ȵȡ�
//
// MessagePump������״̬��������״̬������״̬��˯��״̬��MessagePump��ȡ����������Ϣ��Delegate�����˼�ʱ�����ʱ����,
// ��ôMessagePump�㴦������״̬����MessagePump����������״̬����Delegte��������ʱ������ôMessagePump���ڿ���״̬����
// MessagePumpû���κ���Ϣ��������Ҫ�����ʱ�򣬱�����˯��״̬�Խ�ʡCPU���ġ�
//
// ��������״̬��MessagePump��ÿһ�ִ�����Ϻ���������Կ�ʼ��һ�ִ��������һ��û����Ϣ�ͼ�ʱ����ʱ������Ҫ������ô
// �᳢�Կ�ʼ������ʱ�����������ʱ������Ҫ������ô��������״̬�����û�У���ô������˯��״̬���������״̬��
// ����µ���Ϣ����������µļ�ʱ����ʱ���񱻴�����ô�����л�������״̬���������״̬��û���µĿ���������Ҫ������ô
// �����˯��״̬������˯��״̬��MessagePump���Ա�ScheduleWork��ScheduleDelayedWork�Ȼ��ѽ�������״̬��

class MessagePump
{
public:

	// Delegate��һ���ص��࣬��MessagePump������������������������
	//   DoWork��������ʱ��������������Ҫ�����ܿ�ر���ʼ����
	//   DoDelayedWork��������ʱ���������������ض�ʱ�̱�����next_delayed_message_timeΪ�¸�����ִ�е�ʱ��
	//   DoIdleWork����������ʱ��������������MessagePump���ڿ���״̬ʱ������
	// ���߷���ֵ�������ƣ�����true��ʾ�����˸������񣬷���false��ʾû�и���������Ҫ����
	class Delegate
	{
	public:
		virtual ~Delegate() {}
		virtual bool DoWork() = 0;
		virtual bool DoIdleWork() = 0;
	};

	MessagePump();
	virtual ~MessagePump();

	// ������Ϣѭ��
	virtual void Run(Delegate* delegate) = 0;

	// ֪ͨ�˳���Ϣѭ��
	virtual void Quit() = 0;

	// ֪ͨMessagePump����ʱ����
	// ���Կ��̵߳�������ӿ�
	virtual void ScheduleWork() = 0;
};


