// ClientMessage.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//

#include <iostream>
#include <thread>

#include "FrameworkThread.h"
#include "ThreadManage.h"
#include "Callback.hpp"

using namespace std;

class NetworkThread : public FrameworkThread {
public:
	NetworkThread() : FrameworkThread(" "){
	}

	void Init() {
		ThreadManager::RegisterThread(1);
	}

	void Cleanup() {
		ThreadManager::UnregisterThread();
	}
};

class MainThread : public FrameworkThread, public virtual SupportWeakCallback {
public:
	MainThread() : FrameworkThread(" ") {

	}

	void Init() {
		ThreadManager::RegisterThread(0);
	}

	void Cleanup() {
		ThreadManager::UnregisterThread();
	}

	void start() {
		StdClosure closure = []() {
			cout << "Test" << endl;
		};

		while (true) {
			ThreadManager::PostTask(1, ToWeakCallback(closure));

			std::this_thread::sleep_for(1s);
		}
	}
};

using namespace std;

int main()
{
	MainThread uit;
	uit.Init();

	NetworkThread net;
	net.Start();

	uit.start();
}

