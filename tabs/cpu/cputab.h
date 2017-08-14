#ifndef CPUTAB_H
#define CPUTAB_H

#include "tabwidget.h"
#include <cstdint>

class QStringRef;
class QString;

struct CPUStat {
    uint64_t user;
    uint64_t system;
    uint64_t idle;
};

namespace Ui {
class CPUTab;
}

class CPUTab : public TabWidget
{
    Q_OBJECT

public:
    explicit CPUTab();
    ~CPUTab();

protected slots:
    void refresh();

private:
    QString readKernelAttributeFile(const QString& path);
    QStringRef getCpuInfoValue(const QStringRef& data, const char* name);
    unsigned getCurFreq();
    void readStaticInfo();
    void updateUsageStats();

public:
    static constexpr const char* name = "CPU";

private:
    Ui::CPUTab *ui;
    QString maxFreqStr;
    QVector<CPUStat> lastStats;
};

#endif // CPUTAB_H