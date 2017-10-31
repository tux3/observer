#include "overviewtab.h"
#include "ui_overviewtab.h"

OverviewTab::OverviewTab()
    : ui(new Ui::OverviewTab)
{
    ui->setupUi(this);
}

OverviewTab::~OverviewTab()
{
    delete ui;
}

void OverviewTab::refresh()
{
}
