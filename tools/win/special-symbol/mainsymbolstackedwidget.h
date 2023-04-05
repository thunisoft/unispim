#ifndef MAINSYMBOLSTACKEDWIDGET_H
#define MAINSYMBOLSTACKEDWIDGET_H

#include <QWidget>
#include "characterwidget.h"
#include <QMouseEvent>
namespace Ui {
class MainSymbolStackedWidget;
}

class MainSymbolStackedWidget : public QWidget
{
    Q_OBJECT

public:
    explicit MainSymbolStackedWidget(QWidget *parent = nullptr);
    ~MainSymbolStackedWidget();

    void SaveHistorySymbol();
    void UpdateHistorySymbol();
    void ResetTheHistorySymbol(QString inputStr);
public slots:
    void ResetRecentShowSlot();
    void ShowRecentInputSlot(QString inputStr);
protected:
    bool eventFilter(QObject *target, QEvent *event);

signals:
    void InputSymbol(QString symbol);

private:
    void InitWidget();
private:
    Ui::MainSymbolStackedWidget *ui;
    QVector<QString> m_recentInputVector;
    CharacterWidget* m_recentWidget;

private:
    void UpdateRecentShow();
};

#endif // MAINSYMBOLSTACKEDWIDGET_H
