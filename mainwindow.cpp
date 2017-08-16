#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "tabs/procs/procstab.h"
#include "tabs/overview/overviewtab.h"
#include "tabs/cpu/cputab.h"
#include "tabs/gpu/gputab.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    ui->tabContainer->addTab(new ProcsTab(), ProcsTab::name);
    ui->tabContainer->addTab(new OverviewTab(), OverviewTab::name);
    ui->tabContainer->addTab(new CPUTab(), CPUTab::name);
    ui->tabContainer->addTab(new GPUTab(), GPUTab::name);

    ui->tabContainer->setCurrentIndex(1);
}

MainWindow::~MainWindow()
{
    delete ui;
}
