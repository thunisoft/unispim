#ifndef WORDLIBITEMWIDGET_H
#define WORDLIBITEMWIDGET_H

#include <QWidget>
#include "customize_qwidget.h"

namespace Ui {
class WordlibItemWidget;
}

class WordlibItemWidget : public CustomizeQWidget
{
    Q_OBJECT

public:
    explicit WordlibItemWidget(QWidget *parent = nullptr);
    WordlibItemWidget(QString itemName,QWidget*parent = nullptr);
    ~WordlibItemWidget();

public slots:
    void SlotDeleteItem();

public:
    void RefreshTextShow();

signals:
    void ItemDelete(QString itemName);
private:
    void InitWidget();
    QString m_itemName;

private:
    Ui::WordlibItemWidget *ui;
};

#endif // WORDLIBITEMWIDGET_H
