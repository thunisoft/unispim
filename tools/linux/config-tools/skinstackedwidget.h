#ifndef SKINSTACKEDWIDGET_H
#define SKINSTACKEDWIDGET_H

#include <QWidget>
#include <QButtonGroup>
#include "stylepreviewwidght.h"

#include <QWidget>
#include <QButtonGroup>
#include <QPushButton>

namespace Ui {
class SkinStackedWidget;
class SkinConfigStackedWidget;
}

class SkinStackedWidget : public QWidget
{
    Q_OBJECT

public:
    explicit SkinStackedWidget(QWidget *parent = nullptr);
    ~SkinStackedWidget();

private:
    void InitWidget();
    void ConnectSignalToSlot();
    QColor TranshFromIntToColor(int inputColorValue);
    int TransFormColorToInt(QColor inputColor);
    void resetColors();

public:
    void LoadConfigInfo();

    QStringList GetEnglishFontList();
    QStringList GetChineseFontList();

public slots:
    void ResetConfigInfo();

private slots:
    void OnCandidatesChangedSlot(int index);
    void SlotAlwaysKeepExpandMode(int checkStatus);
    void SlotSetExpandModeLine(int lines);
    void SlotCandidatesDirectChanged(int id,bool status);
    void SlotAllowTabExpand(int flag);

    void SlotToolbarConfigToggled(int id, bool checked);
    void SlotCurrentChineseFontIndexChanged(int index);
    void SlotCurrentFontHeightIndexChanged(int index);
    void SlotCurrentEnglishFontIndexChanged(int index);

    void SlotHideToolbarStateChanged(int checkStatus);
    void SlotOnColorSelectBtnClicked();

    void SlotDrawColorToTheBtn(QPushButton* inputBtn, QColor color);
    void SlotColorItemChanged(int currentIndex);
    void SlotFontSizeChanged(const QString& fontSize);
    void OnFontSizeComboFocusOUt();

protected:
    bool eventFilter(QObject *obj,QEvent*event);

private:
    Ui::SkinConfigStackedWidget *ui;

    QButtonGroup* m_toolbar_group;
    QButtonGroup* m_candidatesGroup;
    StylePreviewWidght* m_stylePreviewWidght;
};

#endif // SKINSTACKEDWIDGET_H
