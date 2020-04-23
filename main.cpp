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

