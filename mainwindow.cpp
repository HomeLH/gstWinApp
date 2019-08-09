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
    this->resize(800,600);

    auto widget = new QWidget;
    auto vlayout = new QVBoxLayout;
    widget->setLayout(vlayout);

    auto player = new gstplayer(widget);
    auto button = new QPushButton(widget);
    button->setText(tr("Play"));
//    button->show();

    vlayout->addWidget(player);
    vlayout->addWidget(button);
    this->setCentralWidget(widget);
    connect(button, &QPushButton::clicked, player, &gstplayer::play);
}

MainWindow::~MainWindow()
{
    delete ui;
}
