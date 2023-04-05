/*********************************************************************
* @file  customlabel.h
* @brief       各种自定义的label控件
* @date        2022-3-12
* @version     V1.0
* @copyright    Copyright (c) 2018-2022
*********************************************************************
*/
#ifndef MOVABLELABEL_H
#define MOVABLELABEL_H
#include <QLabel>
#include <QEvent>

//主要负责软键盘移动的label控件
class MovableLabel : public QLabel
{
    Q_OBJECT
public:
    MovableLabel(QWidget* parent = 0);
    ~MovableLabel();

protected:
    bool event(QEvent *event);
};

//主要负责拼音串显示的label控件
class PinyinLabel : public QLabel
{
    Q_OBJECT
public:
    PinyinLabel(QWidget* parent = 0);
    ~PinyinLabel();
public:
    void clear();
    void set_text(QString content);
signals:
    void content_text_changed(QString content);
};

#endif
