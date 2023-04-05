#ifndef WORDLIBMAINWIDGET_H
#define WORDLIBMAINWIDGET_H

#include <QWidget>

class WordlibShowWidget;

namespace Ui {
class WordlibMainWidget;
}

class WordlibMainWidget : public QWidget
{
    Q_OBJECT

public:
    explicit WordlibMainWidget(QWidget *parent = 0);
    ~WordlibMainWidget();
    void ResetConfigInfo();
    void ReloadWordInfo();

private slots:
    void ShowShowWidget();
    void ShowAddUserWordlibWidget();

private:
    void Init();
    void SetUpCustomize();

    Ui::WordlibMainWidget *ui;
    WordlibShowWidget *m_showWidget;
};

#endif // WORDLIBMAINWIDGET_H
