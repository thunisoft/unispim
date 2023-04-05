#ifndef TOOLBARMENUITEM_H
#define TOOLBARMENUITEM_H

#include <QWidget>
#include <QEvent>
#include <QMouseEvent>
#include <QPaintEvent>

namespace Ui {
class ToolbarMenuItem;
}

class ToolbarMenuItem : public QWidget
{
    Q_OBJECT

public:
    explicit ToolbarMenuItem(QWidget *parent = 0);
    explicit ToolbarMenuItem(QString icon_path,QString text_content,QString expand_icon,QWidget*parent);
    ~ToolbarMenuItem();

public:
    void set_icon(QString icon_path);
    void set_text(QString text_content);
    void set_expand_icon(QString icon_path);
    void set_text_style(QString color_style);

signals:
    void item_clicked();
    void item_hovered_state(int state);
protected:
   bool event(QEvent * e);
   //悬浮进入事件
   void hoverEnter(QHoverEvent * event);
   //悬浮退出事件
   void hoverLeave(QHoverEvent * event);
   //悬浮移动事件
   void hoverMove(QHoverEvent * event);
   void mouseMoveEvent(QMouseEvent *event);
   void mousePressEvent(QMouseEvent *event);
   void mouseReleaseEvent(QMouseEvent *event);
   void paintEvent(QPaintEvent *event);
private:
    Ui::ToolbarMenuItem *ui;
};

#endif // TOOLBARMENUITEM_H
