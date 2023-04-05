#ifndef TABWIDGETABOUT_H
#define TABWIDGETABOUT_H

#include <QWidget>
#include <QMap>

class TabWidget : public QWidget
{
    Q_OBJECT
public:
    explicit TabWidget(QWidget *parent = 0);
    ~TabWidget();
    void SetSymbolStr(const QString& symbol);
    void SetTabName(const QString& tabName);

private:
    void InitTab();
private Q_SLOTS:
    void ClickStrSlot(QString clickStr);

Q_SIGNALS:
    void SelectStr(QString clickStr);

private:
    QString m_tabName;
    QString m_symbol;

};

#endif // TABWIDGETABOUT_H
