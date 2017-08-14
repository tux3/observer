#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "tabs/procs/procstab.h"
#include "tabs/cpu/cputab.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    ui->tabContainer->addTab(new ProcsTab(), ProcsTab::name);
    ui->tabContainer->addTab(new CPUTab(), CPUTab::name);

    ui->tabContainer->setCurrentIndex(1);
}

MainWindow::~MainWindow()
{
    delete ui;
}
