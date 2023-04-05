#ifndef SUGGESTSETWIDGET_H
#define SUGGESTSETWIDGET_H

#include <QWidget>
#include <QMap>
#include <QComboBox>
#include <QPushButton>
#include <QDialog>
#include "ui_suggestset.h"
#include "customize_ui/customize_qwidget.h"

class SuggestsetWidget : public CustomizeQWidget
{

    Q_OBJECT
public:
    explicit SuggestsetWidget(QWidget *parent = 0);
    virtual void reset();

signals:
    void settingChanged(QMap<QString, QString>);

    void CloseSuggestSetWidget();
public slots:

    void onButtonOkClicked();
    void onButtonCancelClicked();
    void onButtonResetClicked();
    void ReloadWidget();
private:


private:
  QComboBox *m_comboBox_suggest_syllable_location;
  QComboBox *m_comboBox_suggest_word_location;
  QComboBox *m_comboBox_suggest_word_count;

  QPushButton *m_buttonReset;
  QPushButton *m_buttonOk;
  QPushButton *m_buttonCancel;
  QPushButton *m_buttonApply;
  Ui::suggestset *ui;
  QMap<QString, QString> m_changed;

public:
    bool CanCloseWindow();
};

#endif // SUGGESTSETWIDGET_H
