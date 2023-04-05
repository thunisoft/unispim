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
#include <QPainter>

class CharacterWidget : public QWidget
{
    Q_OBJECT

public:
    CharacterWidget(QWidget *parent = 0);
    void SetCharacterStr(const QVector<QVector<QString> > inputCharVector);

    void ResetTheSymbol();
    void SetItemSelected(int row, int column);

private:
    QFont GetTabFontName(QString tabname);
    QFont GetFontNameBySymbol(QString symbol,bool &isContain);

public Q_SLOTS:
    void updateFont(const QFont &font);
    void updateSize(const QString &fontSize);

Q_SIGNALS:
    void characterSelected(const QString &character);

protected:
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void paintEvent(QPaintEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void enterEvent(QEvent *event) override;
    void leaveEvent(QEvent *event) override;
private:
    QFont displayFont;
    QVector<QVector<QString> > m_char_vector;
    int m_columnNum;
    double m_cellSize;
    QString m_hoverKey;
    int m_hoverRow = -1;
    int m_hoverColumn = -1;
    QMap<QString,QFont> m_tabFontMap;
    QMap<QString,QFont> m_symbolFontMap;
};

#endif
