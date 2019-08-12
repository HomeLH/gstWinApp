#ifndef SETTINGDIALOG_H
#define SETTINGDIALOG_H

#include <QDialog>

namespace Ui {
class SettingDialog;
}

class SettingDialog : public QDialog
{
    Q_OBJECT

public:
    explicit SettingDialog(QWidget *parent = nullptr);
    ~SettingDialog();
signals:
    void updateUri(QString &uri);
public slots:
    void transUri();

//private slots:
//    void on_buttonBox_accepted();

private:
    Ui::SettingDialog *ui;
};

#endif // SETTINGDIALOG_H
