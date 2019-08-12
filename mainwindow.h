#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "settingdialog.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void on_option_triggered();
//    void settingUri();

private:
    Ui::MainWindow *ui;
    SettingDialog * _settingdialog;


};

#endif // MAINWINDOW_H
