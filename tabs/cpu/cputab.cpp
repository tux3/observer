#include "cputab.h"
#include "ui_cputab.h"
#include <QDebug>
#include <QFile>
#include <QDir>
#include <QGridLayout>
#include <QChart>
#include <QChartView>
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

    unsigned curFreq = getCurFreq();
    ui->cpuFreq->setText(QString("%1 / %2 Ghz").arg(curFreq / 1000./1000., 0, 'f', 2).arg(maxFreqStr));
}

QString CPUTab::readKernelAttributeFile(const QString &path)
{
    QFile f(path);
    if (!f.open(QIODevice::ReadOnly | QIODevice::Truncate))
        return QString();

    return f.readAll();
}

QStringRef CPUTab::getCpuInfoValue(const QStringRef &data, const char *name)
{
    int pos = data.indexOf(':', data.indexOf(name))+2;
    return data.mid(pos, data.indexOf('\n', pos) - pos);
}

unsigned CPUTab::getCurFreq()
{
    QString cpuinfoCurStr = readKernelAttributeFile("/sys/devices/system/cpu/cpu0/cpufreq/cpuinfo_cur_freq");
    if (!cpuinfoCurStr.isNull())
        return cpuinfoCurStr.trimmed().toUInt();

    return readKernelAttributeFile("/sys/devices/system/cpu/cpu0/cpufreq/scaling_cur_freq").trimmed().toUInt();
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

    lastStats.fill({ 0, 0, 0 }, threads);

    QFile maxFreqFile("/sys/devices/system/cpu/cpu0/cpufreq/cpuinfo_max_freq");
    if (!maxFreqFile.open(QIODevice::ReadOnly | QIODevice::Unbuffered)) {
        qWarning() << "Failed to read max CPU frequency";
        maxFreqStr = "?";
    } else {
        maxFreqStr = QString::number(maxFreqFile.readAll().trimmed().toUInt()/1000./1000., 'f', 2);
    }

    QStringList cachesLevels;
    QString cacheDirPath = "/sys/devices/system/cpu/cpu0/cache/";
    QList<QString> entries = QDir(cacheDirPath).entryList({"index?"}, QDir::Dirs, QDir::Name);
    for (QString entry : entries) {
        QString descr = readKernelAttributeFile(cacheDirPath+entry+"/size").trimmed();
        descr += " L" + readKernelAttributeFile(cacheDirPath+entry+"/level").trimmed();
        QString type = readKernelAttributeFile(cacheDirPath+entry+"/type").trimmed();
        if (type == "Data")
            descr += "d";
        else if (type == "Instruction")
            descr += "i";
        cachesLevels += descr;
    }
    ui->caches->setText(cachesLevels.join(" / "));

    int rows = 1;
    for (int i=sqrt(threads); i; --i) {
        if (threads%i == 0) {
            rows = i;
            break;
        }
    }
    int cols = threads / rows;
    for (int i=0; i<rows; ++i) {
        for (int j=0; j<cols; ++j) {
            int n = i*cols+j;
            QChart* chart = new QChart();
            chart->setTitle(QString("CPU %1").arg(n));
            ui->chartsLayout->addWidget(new QChartView(chart), i, j);
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

    QVector<float> usages;
    for (int i = 0; i<stats.size(); ++i) {
        auto user = stats[i].user - lastStats[i].user;
        auto system = stats[i].system - lastStats[i].system;
        auto idle = stats[i].idle - lastStats[i].idle;
        usages += 100. * (user+system) / (user+system+idle);
    }

    ui->cpuPercents->setText(QString("%1 %").arg(usages[0], 0, 'f', 1));
    lastStats = stats;
}
