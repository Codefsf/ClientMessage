// ClientMessage.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//

#include <iostream>
#include <thread>

#include "FrameworkThread.h"
#include "ThreadManage.h"
#include "Callback.hpp"

#include "mainwindow.h"
#include <QApplication>

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

class Callback : public virtual SupportWeakCallback {
public:
	Callback() {}
	~Callback() {
		cout << "---------------------decall" << endl;
	}

	void Test(int count) {
		cout << "Thread id:" << this_thread::get_id() << " " << this << endl;


		cout << "Test" << count << endl;
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

	void Test(int count) {

		cout << "Test" << endl;
	}

	void start() {
		int num = 0;
		Callback* call = new Callback;
		auto clu = Bind(&Callback::Test, call, 55);
		while (true) {
			ThreadManager::PostTask(1, clu);

			std::this_thread::sleep_for(1s);

			num++;

			if (num == 5 && call != nullptr) {
				delete call;
				call = nullptr;
			}
		}
	}
};

using namespace std;

int main(int argc, char *argv[])
{
	cout << "Main thread id:" << this_thread::get_id() << endl;

    QApplication app(argc, argv);

	NetworkThread net;
	net.Start();

    MainWindow win;
    win.show();

    app.exec();
}

