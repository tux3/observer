#include "cputab.h"
#include "ui_cputab.h"

CPUTab::CPUTab()
    : ui(new Ui::CPUTab)
{
    ui->setupUi(this);
}

CPUTab::~CPUTab()
{
    delete ui;
}

void CPUTab::refresh()
{
}
