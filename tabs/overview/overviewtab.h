#ifndef OVERVIEWTAB_H
#define OVERVIEWTAB_H

#include "tabwidget.h"

namespace Ui {
class OverviewTab;
}

class OverviewTab : public TabWidget
{
    Q_OBJECT

public:
    explicit OverviewTab();
    ~OverviewTab();

protected slots:
    void refresh();

public:
    static constexpr const char* name() {return "Overview";}

private:
    Ui::OverviewTab *ui;
};

#endif // OVERVIEWTAB_H
