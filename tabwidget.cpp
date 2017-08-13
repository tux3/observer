#include "tabwidget.h"
#include <QDebug>

QTimer TabWidget::refreshTimer;

TabWidget::TabWidget()
    : visible{false}
{
    refreshTimer.start(DEFAULT_REFRESH_INTERVAL);

    connect(&refreshTimer, &QTimer::timeout, this, &TabWidget::refreshIfVisible);
}

void TabWidget::setRefreshInterval(unsigned refreshInterval)
{
    refreshTimer.setInterval(refreshInterval);
}

void TabWidget::showEvent(QShowEvent*)
{
    visible = true;
    refresh();
}

void TabWidget::hideEvent(QHideEvent *)
{
    visible = false;
}
