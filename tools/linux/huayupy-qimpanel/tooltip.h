#ifndef TOOLTIP_H
#define TOOLTIP_H

#include "customize_qwidget.h"
#include <QString>
#include <QPaintEvent>
#include <QFont>

class ToolTip : CustomizeQWidget
{
    Q_OBJECT
public:
    static ToolTip* Instance();
    void ShowTip(const QString& tip, int parent_bottom_, int parent_height_);
    void Hide();

private:
    ToolTip(QWidget *parent = NULL);
    void CalWindowSize(const QString& tip, const QFont* font);
    void InitFont();
    void Move(int bottom_limit, int top_limit);

private slots:
    void paintEvent(QPaintEvent *e);
private:
    static ToolTip* m_tool_tip;


    QString m_tip_text;
    QFont* m_font;
    int m_text_width;
    int m_text_height;
};

#endif // TOOLTIP_H
