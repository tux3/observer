#ifndef CPUTAB_H
#define CPUTAB_H

#include "tabwidget.h"

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

public:
    static constexpr const char* name = "CPU";

private:
    Ui::CPUTab *ui;
};

#endif // CPUTAB_H
