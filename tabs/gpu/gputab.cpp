#include "gputab.h"
#include "ui_gputab.h"
#include <QAreaSeries>
#include <QChart>
#include <QChartView>
#include <QDebug>
#include <QGraphicsLayout>
#include <QLineSeries>
#include <QValueAxis>
#include <cstring>

using namespace QtCharts;

GPUTab::GPUTab(unsigned gpuNum)
    : ui(new Ui::GPUTab)
    , tabName{ 0 }
{
    ui->setupUi(this);

    auto tabNameStr = QString("GPU %1").arg(gpuNum).toStdString();
    memcpy(tabName, tabNameStr.c_str(), tabNameStr.length());

    nvmlDeviceGetHandleByIndex(gpuNum, &device);

    char name[64];
    nvmlDeviceGetName(device, name, 64);
    ui->gpuName->setText(name);

    QChart* chart = new QChart();
    QValueAxis* xAxis = new QValueAxis();
    xAxis->setRange(0, timeResolution);
    xAxis->setLabelsVisible(false);
    xAxis->setLineVisible(false);
    xAxis->setTickCount(7);
    xAxis->setGridLineColor(Qt::darkGray);

    QValueAxis* yAxis = new QValueAxis();
    yAxis->setRange(0, 100);
    yAxis->setLabelsVisible(true);
    yAxis->setLineVisible(false);
    yAxis->setTickCount(11);
    yAxis->setGridLineColor(Qt::darkGray);

    chart->setTitle("GPU Usage");
    chart->legend()->hide();
    chart->setBackgroundRoundness(0);
    chart->setMargins(QMargins(0, 0, 0, -15));
    chart->layout()->setContentsMargins(0, 0, 0, 0);
    chart->setBackgroundVisible(false);

    QChartView* chartView = new QChartView(chart);
    chartView->setRenderHint(QPainter::HighQualityAntialiasing);
    ui->verticalLayout->insertWidget(1, chartView);

    usageSeries = new QLineSeries();
    chart->addSeries(usageSeries);
    chart->addAxis(xAxis, Qt::AlignBottom);
    chart->addAxis(yAxis, Qt::AlignLeft);
    usageSeries->attachAxis(xAxis);
    usageSeries->attachAxis(yAxis);
}

GPUTab::~GPUTab()
{
    delete ui;
}

void GPUTab::refresh()
{
    updateUsageGraph();

    if (!isVisible())
        return;

    updateStats();
}

void GPUTab::updateUsageGraph()
{
    nvmlUtilization_t utilization;
    nvmlDeviceGetUtilizationRates(device, &utilization);

    QVector<QPointF> points = usageSeries->pointsVector();
    double nextX = qMax(points.size(), timeResolution);

    if (points.size() >= timeResolution)
        points.removeFirst();
    for (QPointF& point : points)
        point.rx() -= 1;

    points.push_back({ nextX, (double)utilization.gpu });
    usageSeries->replace(points);
}

void GPUTab::updateStats()
{
    nvmlUtilization_t utilization;
    nvmlDeviceGetUtilizationRates(device, &utilization);
    ui->usagePercents->setText(QString("%1 %").arg(utilization.gpu));

    unsigned powerUsage;
    nvmlDeviceGetPowerUsage(device, &powerUsage);
    ui->powerUsage->setText(QString("%1 W").arg((double)powerUsage / 1000, 0, 'f', 2));

    unsigned temperature;
    nvmlDeviceGetTemperature(device, NVML_TEMPERATURE_GPU, &temperature);
    ui->temperature->setText(QString("%1°C").arg(temperature));

    nvmlMemory_t memInfo;
    nvmlDeviceGetMemoryInfo(device, &memInfo);
    ui->memoryUsed->setText(QString("%1 / %2 MiB").arg(memInfo.used / 1024 / 1024).arg(memInfo.total / 1024 / 1024));

    unsigned clockSpeed;
    nvmlDeviceGetClockInfo(device, NVML_CLOCK_GRAPHICS, &clockSpeed);
    ui->graphicsClock->setText(QString("%1 Mhz").arg(clockSpeed));
    nvmlDeviceGetClockInfo(device, NVML_CLOCK_MEM, &clockSpeed);
    ui->memoryClock->setText(QString("%1 Mhz").arg(clockSpeed));
}
