#include "settingdialog.h"
#include "ui_settingdialog.h"
#include <QDebug>

SettingDialog::SettingDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::SettingDialog)
{
    ui->setupUi(this);
    connect(this, &SettingDialog::accepted, this, &SettingDialog::transUri);
}

SettingDialog::~SettingDialog()
{
    delete ui;
    qDebug("close");
}

void SettingDialog::transUri()
{
    qDebug() << "transuri accepted!";
    QString uri {ui->lineEdit->text()};
    emit updateUri(uri);
}

//void SettingDialog::on_buttonBox_accepted()
//{
//    qDebug() << "buttonbox accepted!";

//    QString uri {ui->lineEdit->text()};
//    qDebug() << uri;
//    emit updateUri(uri);
//}
