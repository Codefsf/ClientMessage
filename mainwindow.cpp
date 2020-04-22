#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "ThreadManage.h"

#include <iostream>

using namespace std;

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_pushButton_clicked()
{
    auto clu = Bind(&MainWindow::callback, this, 55);
    ThreadManager::PostTask(1, clu);
}

void MainWindow::callback(int count)
{
    cout << "--------------" << count << endl;;
}
