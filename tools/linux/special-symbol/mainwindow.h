#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QQueue>
#include <QLabel>
#include "characterwidget.h"
#include <QCloseEvent>
#include <map>

using namespace std;

namespace Ui {
class MainWindow;
}
class TabWidget;

enum SYMBOL
{
    SPECAIL_,
    PUNC_,
    NUM_ORDER_,
    NUM_UNIT_,
    GREECE_,
    PIN_,
    CN_,
    ENG_,
    JAP_,
    KOR_,
    RUS_,
    TAB_
};

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private Q_SLOTS:
    void ShowRecentInputSlot(QString inputStr);
    void ResetRecentShowSlot();
    void ToShow();

    void Button1Cliked();
    void Button2Cliked();
    void Button3Cliked();
    void Button4Cliked();
    void Button5Cliked();
    void Button6Cliked();
    void Button7Cliked();
    void Button8Cliked();
    void Button9Cliked();
    void Button10Cliked();
    void Button11Cliked();
    void Button12Cliked();

    void ButtonCliked(SYMBOL symbol);
private:
    void InitInputService();
    void UnInitInputService();
    void UpdateRecentShow();

    void UpdateHistorySymbol();
    void SaveHistorySymbol();

    void ResetTheHistorySymbol(QString inputStr);
    void ConnectButton(const int button_index, const int tab_index, const QString& button_name);

protected:
    void closeEvent(QCloseEvent *event);
Q_SIGNALS:
    void commit(QString str);

private:
    Ui::MainWindow *ui;
    QVector<QString> m_recentInputVector;
    CharacterWidget* m_recentWidget;
    map<SYMBOL, int > m_symbol_index_map;
//    InputService* m_inputService;
};

#endif // MAINWINDOW_H
