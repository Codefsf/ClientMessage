#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "Callback.hpp"
#include <QTimer>

namespace Ui {
class MainWindow;
}

class QNetworkReply;

class MainWindowModel : public QObject, public SupportWeakCallback
{
    Q_OBJECT

public:
    explicit MainWindowModel(QWidget *parent = nullptr) : QObject(parent){

    }
    ~MainWindowModel(){}

    void call();
    void callBack();

signals:
    void dataChanged(const QString &str);

public slots:
    void netReply(QNetworkReply *reply);
};

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void on_pushButton_clicked();
    void callback(const QString &str);

private:
    Ui::MainWindow *ui;
    QTimer m_timer;
    MainWindowModel *m_model;
};

#endif // MAINWINDOW_H
