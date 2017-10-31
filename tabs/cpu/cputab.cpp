#include "cputab.h"
#include "ui_cputab.h"
#include <QAreaSeries>
#include <QChart>
#include <QChartView>
#include <QDebug>
#include <QDir>
#include <QFile>
#include <QGraphicsLayout>
#include <QGridLayout>
#include <QLineSeries>
#include <QValueAxis>
#include <cmath>

using namespace QtCharts;

CPUTab::CPUTab()
    : ui(new Ui::CPUTab)
{
    ui->setupUi(this);

    readStaticInfo();
}

CPUTab::~CPUTab()
{
    delete ui;
}

void CPUTab::refresh()
{
    updateUsageStats();

    if (!isVisible())
        return;

    uint64_t curFreq = 0;
    for (int i = 0; i < lastStats.size() - 1; ++i) {
        curFreq += getCurFreq(i);
    }
    curFreq /= lastStats.size() - 1;
    ui->cpuFreq->setText(QString("%1 / %2 Ghz").arg(curFreq / 1000. / 1000., 0, 'f', 2).arg(maxFreqStr));

    updateSensors();
}

QString CPUTab::readKernelAttributeFile(const QString& path)
{
    QFile f(path);
    if (!f.open(QIODevice::ReadOnly | QIODevice::Truncate))
        return QString();

    return f.readAll();
}

QStringRef CPUTab::getCpuInfoValue(const QStringRef& data, const char* name)
{
    int pos = data.indexOf(':', data.indexOf(name)) + 2;
    return data.mid(pos, data.indexOf('\n', pos) - pos);
}

unsigned CPUTab::getCurFreq(unsigned cpuNum)
{
    QString cpuinfoCurStr = readKernelAttributeFile(QString("/sys/devices/system/cpu/cpu%1/cpufreq/cpuinfo_cur_freq").arg(cpuNum));
    if (!cpuinfoCurStr.isNull())
        return cpuinfoCurStr.trimmed().toUInt();

    return readKernelAttributeFile(QString("/sys/devices/system/cpu/cpu%1/cpufreq/scaling_cur_freq").arg(cpuNum)).trimmed().toUInt();
}

void CPUTab::readStaticInfo()
{
    QFile file("/proc/cpuinfo");
    if (!file.open(QIODevice::ReadOnly | QIODevice::Unbuffered)) {
        qWarning() << "Failed to read /proc/cpuinfo !";
        return;
    }

    QString strSource = file.readAll();
    QStringRef str(&strSource);

    ui->cpuName->setText(getCpuInfoValue(str, "model name").toString());

    int cores = getCpuInfoValue(str, "cpu cores").toInt();
    QStringRef lastCoreStr = str.mid(str.lastIndexOf("\n\nprocessor"));
    int threads = getCpuInfoValue(lastCoreStr, "processor").toInt() + 1;
    ui->cores->setText(QString("%1c / %2t").arg(cores).arg(threads));

    QFile maxFreqFile("/sys/devices/system/cpu/cpu0/cpufreq/cpuinfo_max_freq");
    if (!maxFreqFile.open(QIODevice::ReadOnly | QIODevice::Unbuffered)) {
        qWarning() << "Failed to read max CPU frequency";
        maxFreqStr = "?";
    } else {
        maxFreqStr = QString::number(maxFreqFile.readAll().trimmed().toUInt() / 1000. / 1000., 'f', 2);
    }

    QStringList cachesLevels;
    QString cacheDirPath = "/sys/devices/system/cpu/cpu0/cache/";
    QList<QString> entries = QDir(cacheDirPath).entryList({ "index?" }, QDir::Dirs, QDir::Name);
    for (QString entry : entries) {
        QString descr = readKernelAttributeFile(cacheDirPath + entry + "/size").trimmed();
        descr += " L" + readKernelAttributeFile(cacheDirPath + entry + "/level").trimmed();
        QString type = readKernelAttributeFile(cacheDirPath + entry + "/type").trimmed();
        if (type == "Data")
            descr += "d";
        else if (type == "Instruction")
            descr += "i";
        cachesLevels += descr;
    }
    ui->caches->setText(cachesLevels.join(" / "));

    int rows = 1;
    for (int i = sqrt(threads); i; --i) {
        if (threads % i == 0) {
            rows = i;
            break;
        }
    }
    int cols = threads / rows;
    for (int i = 0; i < rows; ++i) {
        for (int j = 0; j < cols; ++j) {
            QChart* chart = new QChart();
            QValueAxis* xAxis = new QValueAxis();
            xAxis->setRange(0, timeResolution);
            xAxis->setLabelsVisible(false);
            xAxis->setLineVisible(false);
            xAxis->setTickCount(7);
            xAxis->setGridLineColor(Qt::darkGray);

            QValueAxis* yAxis = new QValueAxis();
            yAxis->setRange(0, 100);
            yAxis->setLabelsVisible(false);
            yAxis->setLineVisible(false);
            yAxis->setTickCount(6);
            yAxis->setGridLineColor(Qt::darkGray);

            chart->legend()->hide();
            chart->setBackgroundRoundness(0);
            // The axes add a LOT of margin when visible, even when they don't have anything to show!
            // I can't find a way to remove it completely even with the margins at 0,
            // so we overstep a bit into the negatives to get a reasonnably small margin
            chart->setMargins(QMargins(-30, 0, 0, -15));
            chart->layout()->setContentsMargins(0, 0, 0, 0);
            chart->setBackgroundVisible(false);

            QChartView* chartView = new QChartView(chart);
            chartView->setRenderHint(QPainter::HighQualityAntialiasing);
            ui->chartsLayout->addWidget(chartView, i, j);

            QLineSeries* series = new QLineSeries();
            chart->addSeries(series);
            chart->addAxis(xAxis, Qt::AlignBottom);
            chart->addAxis(yAxis, Qt::AlignLeft);
            series->attachAxis(xAxis);
            series->attachAxis(yAxis);
            series->setBrush(Qt::red);

            chartSeries.append(series);
        }
    }
}

void CPUTab::updateUsageStats()
{
    QString statsStr = readKernelAttributeFile("/proc/stat");
    if (statsStr.isNull()) {
        qWarning() << "Failed to open /proc/stat";
        return;
    }

    QVector<CPUStat> stats;
    QVector<QStringRef> lines = statsStr.splitRef('\n');
    for (QStringRef line : lines) {
        if (!line.startsWith("cpu"))
            break;
        QVector<QStringRef> params = line.split(' ', QString::SkipEmptyParts);
        stats += {
            params[1].toULongLong(),
            params[3].toULongLong(),
            params[4].toULongLong(),
        };
    }

    if (lastStats.isEmpty()) {
        ui->cpuPercents->setText(QString("0 %"));
        lastStats = stats;
        return;
    }

    QVector<float> usages;
    for (int i = 0; i < stats.size(); ++i) {
        auto user = stats[i].user - lastStats[i].user;
        auto system = stats[i].system - lastStats[i].system;
        auto idle = stats[i].idle - lastStats[i].idle;
        usages += 100. * (user + system) / (user + system + idle);
    }

    ui->cpuPercents->setText(QString("%1 %").arg(usages[0], 0, 'f', 1));
    lastStats = stats;

    if (usages.size() - 1 != chartSeries.size()) {
        qCritical() << "We have" << chartSeries.size() << "CPU thread charts, but found" << usages.size() - 1 << "threads in the stats!";
        return;
    }
    for (int i = 0; i < usages.size() - 1; ++i) {
        QXYSeries* series = chartSeries[i];
        QVector<QPointF> points = series->pointsVector();

        double nextX = qMax(points.size(), timeResolution);
        double nextY = usages[i + 1];

        if (points.size() >= timeResolution)
            points.removeFirst();
        for (QPointF& point : points)
            point.rx() -= 1;

        points.push_back({ nextX, nextY });
        series->replace(points);
    }
}

void CPUTab::updateSensors()
{
    // NOTE: The values we report are probably going to be thoroughly wrong on a wide swath or hardware,
    // but it Works On My Machine™!
    // TODO: Put this behind an opt-in, and have the opt-in warn that it's as unreliable as it gets

    // TODO: Only accept the voltage from the input named Vcore or VcoreA, the name usually comes from the libsensors config file
    // Same with the temperature, look for a sensor called "CPU Temp", "CPU0 Temp" or "Core0 Temp"

    QString voltageStr = readKernelAttributeFile("/sys/class/hwmon/hwmon0/in0_input");
    if (voltageStr.isNull()) {
        ui->voltage->setText("Unknown");
    } else {
        float voltage = voltageStr.toUInt() / 1000.;
        ui->voltage->setText(QString("%1 V").arg(voltage, 0, 'f', 2));
    }
}
