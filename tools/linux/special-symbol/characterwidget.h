#ifndef CHARACTERWIDGET_H
#define CHARACTERWIDGET_H

#include <QFont>
#include <QPoint>
#include <QSize>
#include <QString>
#include <QWidget>
#include <QPaintEvent>
#include <QMouseEvent>
#include <QVector>


class CharacterWidget : public QWidget
{
    Q_OBJECT

public:
    CharacterWidget(QWidget *parent = 0);
    void SetCharacterStr(const QVector<QVector<QString> > inputCharVector);

public Q_SLOTS:
    void updateFont(const QFont &font);
    void updateSize(const QString &fontSize);

Q_SIGNALS:
    void characterSelected(const QString &character);

protected:
    void mouseMoveEvent(QMouseEvent *event);
    void mousePressEvent(QMouseEvent *event);
    void paintEvent(QPaintEvent *event);

private:
    QFont displayFont;
    QVector<QVector<QString> > m_char_vector;
    int m_columnNum;
    int m_cellSize;
};

#endif
