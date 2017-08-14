#ifndef TABWIDGET_H
#define TABWIDGET_H

#include <QWidget>
#include <QTimer>

class QShowEvent;
class QHideEvent;

class TabWidget : public QWidget
{
    Q_OBJECT

public:
    TabWidget();
    virtual ~TabWidget() = default;

    static void setRefreshInterval(unsigned refreshInterval);

protected slots:
    virtual void refresh() = 0;

protected:
    virtual void showEvent(QShowEvent*) override;
    virtual void hideEvent(QHideEvent*) override;
    inline bool isVisible() const { return visible; }

private:
    static constexpr const unsigned DEFAULT_REFRESH_INTERVAL = 1000;
    static QTimer refreshTimer;
    bool visible;
};

#endif // TABWIDGET_H
