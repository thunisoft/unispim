#ifndef PHRASEOPTIONWIDGET_H
#define PHRASEOPTIONWIDGET_H

#include <QWidget>

enum BUTTON_TYPE
{
    EDIT_PHRASE,
    DELETE_PHRASE
};

namespace Ui {
class PhraseOptionWidget;
}

class PhraseOptionWidget : public QWidget
{
    Q_OBJECT

public:
    explicit PhraseOptionWidget(int phrase_index, QWidget *parent = 0);
    ~PhraseOptionWidget();
signals:
    void OptionPhraseButtonClicked(int button_index, int phrase_index);
private slots:
    void OnEditButtonClicked();
    void OnDeleteButtonClicked();
private:

    void Init();

    int m_phrase_index;
    Ui::PhraseOptionWidget *ui;
};

#endif // PHRASEOPTIONWIDGET_H
