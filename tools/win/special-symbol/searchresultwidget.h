#ifndef SEARCHRESULTWIDGET_H
#define SEARCHRESULTWIDGET_H

#include <QWidget>
#include "characterwidget.h"

namespace Ui {
class SearchResultWidget;
}

class SearchResultWidget : public QWidget
{
    Q_OBJECT

public:
    explicit SearchResultWidget(QWidget *parent = nullptr);
    ~SearchResultWidget();
public:
    bool UpdateSearchResult(QVector<QString> &symbolVector);
private:
    void InitWidght();
signals:
    void returnMainWindow();
    void clickSearchSymbol(QString symbol);
private slots:
    void ReturnBtnClickSlot();
    void ClickStrSlot(QString symbol);

private:
    Ui::SearchResultWidget *ui;
    CharacterWidget* m_resultWidght;
};

#endif // SEARCHRESULTWIDGET_H
