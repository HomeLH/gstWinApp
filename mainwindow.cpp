#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "gstplayer.h"
#include <QPushButton>
#include <QVBoxLayout>


MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    this->resize(400,300);

    auto widget = new QWidget;
    auto vlayout = new QVBoxLayout;
    widget->setLayout(vlayout);

    auto player = new gstplayer(widget);
//    auto button = new QPushButton(widget);
//    button->setText(tr("Play"));
//    vlayout->addWidget(button);

    vlayout->addWidget(player);

    this->setCentralWidget(widget);
//    connect(button, &QPushButton::clicked, player, &gstplayer::play);



    _settingdialog = new SettingDialog(this);
    _settingdialog->hide();
    // 更新视频uri
    connect(_settingdialog, &SettingDialog::updateUri, player, &gstplayer::setUri);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_option_triggered()
{
    _settingdialog->open();
//    if(_settingdialog->exec() == QDialog::ac;
}
