#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "ThreadManage.h"

#include <iostream>
#include <functional>
#include <thread>
#include <QPainter>
#include <QBrush>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QDebug>
#include <QEventLoop>

using namespace std;

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    m_model = new MainWindowModel;

    connect(m_model, &MainWindowModel::dataChanged,
            this, &MainWindow::callback);
    connect(&m_timer, &QTimer::timeout,
            this, &MainWindow::on_pushButton_clicked);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_pushButton_clicked()
{
    m_model->call();
}

void MainWindow::callback(const QString &str)
{
    cout << "Thread" << this_thread::get_id() << endl;;
    ui->label_2->text().isEmpty() ?
                ui->label_2->setText(str) :
                ui->label_2->clear();
}

void MainWindowModel::call()
{
    ThreadManager::PostTask(1, Bind(&MainWindowModel::callBack, this));
}

void MainWindowModel::callBack()
{
    QNetworkAccessManager *manager = new QNetworkAccessManager();
    connect(manager, &QNetworkAccessManager::finished,
            [=](QNetworkReply* re){
        cout << "Thread callback" << this_thread::get_id() << endl;;

        emit dataChanged(re->readAll());
    });


    manager->get(QNetworkRequest(QUrl("http://www.sina.com")));
}

void MainWindowModel::netReply(QNetworkReply *reply)
{
    cout << "--------------reply" << this_thread::get_id() << endl;
        QByteArray arr = reply->readAll();
        qDebug() << arr;
}
