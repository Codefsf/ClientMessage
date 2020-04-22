#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "Callback.hpp"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow, public SupportWeakCallback
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void on_pushButton_clicked();
    void callback(int count);

private:
    Ui::MainWindow *ui;
};

#endif // MAINWINDOW_H
