#include "tabwidgetbase.h"

TabWidgetBase::TabWidgetBase(const QString& name, QWidget *parent)
    : QWidget(parent)
    , m_name(name)
{
}

void TabWidgetBase::onCheckBoxStateChanged(const QString &objectName, int state)
{
    if (state == Qt::Checked)
    {
        if (m_changed.contains(objectName) && m_changed[objectName] == "Qt::Unchecked")
        {
            m_changed.remove(objectName);
        }
        else
        {
            m_changed[objectName] = "Qt::Checked";
        }
    }
    else if (state == Qt::Unchecked)
    {
        if (m_changed.contains(objectName) && m_changed[objectName] == "Qt::Checked")
        {
            m_changed.remove(objectName);
        }
        else
        {
            m_changed[objectName] = "Qt::Unchecked";
        }
    }
}

