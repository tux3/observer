#ifndef GPUTAB_H
#define GPUTAB_H

#include "tabwidget.h"

namespace Ui {
class GPUTab;
}

class GPUTab : public TabWidget
{
    Q_OBJECT

public:
    explicit GPUTab();
    ~GPUTab();

protected slots:
    void refresh();

public:
    static constexpr const char* name = "GPU";

private:
    Ui::GPUTab *ui;
};

#endif // GPUTAB_H
