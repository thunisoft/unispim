#ifndef TABWIDGETBASE_H
#define TABWIDGETBASE_H

#include <QWidget>
#include <QMap>
#include <QDebug>

class ConfigBus;

class TabWidgetBase : public QWidget
{
    Q_OBJECT
public:
    explicit TabWidgetBase(const QString& name, QWidget *parent = 0);

    virtual QMap<QString, QString> apply() = 0;
    virtual void reset() = 0;

    const QString &name() const { return m_name; }

signals:

protected:
    void onCheckBoxStateChanged(const QString &objectName, int state);

protected:
    QMap<QString, QString> m_memo;
    QMap<QString, QString> m_changed;

private:
    QString m_name;
};

#endif // TABWIDGETBASE_H
