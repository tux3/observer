#include "gputab.h"
#include "ui_gputab.h"

GPUTab::GPUTab()
    : ui(new Ui::GPUTab)
{
    ui->setupUi(this);
}

GPUTab::~GPUTab()
{
    delete ui;
}

void GPUTab::refresh()
{

}
