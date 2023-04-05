#ifndef SKINCONFIGSTACKEDWIDGET_H
#define SKINCONFIGSTACKEDWIDGET_H

#include <QWidget>
#include <QButtonGroup>
#include <QPushButton>
#include <QLineEdit>
#include <QFocusEvent>
#include <QFileSystemWatcher>
#include "customize_ui/basestackedwidget.h"

namespace Ui {
class SkinConfigStackedWidget;
}

class MyLineEdit final : public QLineEdit
{
    Q_OBJECT

public:
    MyLineEdit() = default;
    ~MyLineEdit() = default;

signals:
    void focus_out();

protected:

    void focusInEvent(QFocusEvent *e)override
    {
        QLineEdit::focusInEvent(e);
    }
    void focusOutEvent(QFocusEvent *e)override
    {
        emit focus_out();
        QLineEdit::focusOutEvent(e);
    }

};

class StylePreviewWidght;
class SkinConfigStackedWidget : public BaseStackedWidget
{
    Q_OBJECT

    using my_line_eidt = MyLineEdit;

public:
    explicit SkinConfigStackedWidget(QWidget *parent = nullptr);
    ~SkinConfigStackedWidget();

private:
    void InitWidget();
    void ConnectSignalToSlot();
    QColor TranshFromIntToColor(int inputColorValue);
    int TransFormColorToInt(QColor inputColor);
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

    void SlotToolbarStatusChanged(int id);
    void SlotCurrentChineseFontIndexChanged(int index);
    void SlotCurrentFontHeightIndexChanged(int index);
    void SlotCurrentEnglishFontIndexChanged(int index);

    void SlotHideToolbarStateChanged(int checkStatus);
    void SlotOnColorSelectBtnClicked();

    void SlotDrawColorToTheBtn(QPushButton* inputBtn, QColor color);
    void SlotColorItemChanged(int currentIndex);
    void SlotFontSizeChanged(const QString& fontSize);
    void SlotConfigFileChanged(QString file_path);

protected:
    bool eventFilter(QObject *obj,QEvent*event);
private:
    Ui::SkinConfigStackedWidget *ui;
    QButtonGroup* m_toolbar_group;
    QButtonGroup* m_candidatesGroup;
    StylePreviewWidght* m_stylePreviewWidght;
    my_line_eidt* m_font_lineedit = nullptr;
    QFileSystemWatcher* m_file_watcher = nullptr;

};

#endif // SKINCONFIGSTACKEDWIDGET_H
