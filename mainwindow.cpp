#include "mainwindow.h"
#include "tabs/cpu/cputab.h"
#include "tabs/gpu/gputab.h"
#include "tabs/overview/overviewtab.h"
#include "tabs/procs/procstab.h"
#include "ui_mainwindow.h"
#include <QDebug>
#include <nvml.h>

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    ui->tabContainer->addTab(new ProcsTab(), ProcsTab::name());
    ui->tabContainer->addTab(new OverviewTab(), OverviewTab::name());
    ui->tabContainer->addTab(new CPUTab(), CPUTab::name());

    if (nvmlInit() != NVML_SUCCESS) {
        qWarning() << "Failed to initialize Nvidia NVML, GPU monitoring won't be available";
    } else {
        unsigned numDevices = 0;
        if (nvmlDeviceGetCount(&numDevices) != NVML_SUCCESS) {
            qWarning() << "Failed to get number of GPUs, assuming zero";
        }

        for (decltype(numDevices) i = 0; i < numDevices; ++i) {
            auto gpuTab = new GPUTab(i);
            ui->tabContainer->addTab(gpuTab, gpuTab->name());
        }
    }
}

MainWindow::~MainWindow()
{
    delete ui;

    nvmlShutdown();
}
