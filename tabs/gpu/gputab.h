#ifndef GPUTAB_H
#define GPUTAB_H

#include "tabwidget.h"
#include <QXYSeries>
#include <nvml.h>

namespace Ui {
class GPUTab;
}

class GPUTab : public TabWidget {
    Q_OBJECT

public:
    explicit GPUTab(unsigned gpuNum);
    ~GPUTab();
    const char* name() { return tabName; }

protected slots:
    void refresh();

private:
    void updateUsageGraph();
    void updateStats();

private:
    static constexpr const int timeResolution = 60;

    Ui::GPUTab* ui;
    char tabName[8];

    nvmlDevice_t device;
    QtCharts::QXYSeries* usageSeries;
};

#endif // GPUTAB_H
