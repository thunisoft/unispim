#include <QtWidgets>
#include "characterwidget.h"


CharacterWidget::CharacterWidget(QWidget *parent)
    : QWidget(parent),
      m_columnNum(12)
{
    m_cellSize = 35;
    setMouseTracking(true);
    updateFont(QFont("KaiXinSong2.1"));
    updateSize(QString("12"));
}
void CharacterWidget::updateFont(const QFont &font)
{
    displayFont.setFamily(font.family());
    adjustSize();
    update();
}

void CharacterWidget::updateSize(const QString &fontSize)
{
    displayFont.setPointSize(fontSize.toInt());
    adjustSize();
    update();
}

void CharacterWidget::mouseMoveEvent(QMouseEvent *event)
{
    QPoint widgetPosition = mapFromGlobal(event->globalPos());
    int row = widgetPosition.y()/m_cellSize;
    int column = widgetPosition.x()/m_cellSize;
    int rowSize = m_char_vector.size();
    if(row >= rowSize || row < 0)
    {
        return;
    }
    QVector<QString> rowStrVector = m_char_vector.at(row);
    if(column >= rowStrVector.size() || column < 0)
    {
        return;
    }
    QString key = rowStrVector.at(column);
    if(key == " ")
    {
        return;
    }

    QString text = QString("<p>字符: <span style=\"font-size: 24pt; font-family: %1\">").arg(displayFont.family())
                  + key
                  + QString("</span><p>码点: 0x")
                  + QString::number(key.at(0).unicode(), 16);
    QToolTip::showText(event->globalPos(), text, this);
}

void CharacterWidget::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        QPoint widgetPosition = mapFromGlobal(event->globalPos());
        int row = widgetPosition.y()/m_cellSize;
        int column = widgetPosition.x()/m_cellSize;
        int rowSize = m_char_vector.size();
        if(row >= rowSize || row <0)
        {
            return;
        }
        QVector<QString> rowStrVector = m_char_vector.at(row);
        if(column >= rowStrVector.size() || column < 0)
        {
            return;
        }
        QString key = rowStrVector.at(column);
        if(key != " ")
        {
            Q_EMIT characterSelected(key);
        }
        update();
    }
    else
        QWidget::mousePressEvent(event);
}

void CharacterWidget::paintEvent(QPaintEvent *event)
{
    //先画背景框
    QPainter painter(this);
    painter.fillRect(event->rect(), QBrush(Qt::white));
    painter.setFont(displayFont);

    QRect redrawRect = this->rect();
    int top = redrawRect.top();
    int left = redrawRect.left();

    int rows = m_char_vector.size();


    painter.setPen(QPen(Qt::gray));
    for (int row = 0; row < rows; ++row)
    {
        for (int column = 0; column < m_columnNum; ++column)
        {
            painter.drawRect(left + column*m_cellSize, top + row*m_cellSize, m_cellSize, m_cellSize);
        }
    }

    //再在背景框里面画文字
    QFontMetrics fontMetrics(displayFont);
    painter.setPen(QPen(Qt::black));
    for (int row = 0; row < rows; ++row)
    {
        int columnSize = m_char_vector.at(row).length();
        if(columnSize == 0)
        {
            return;
        }
        for (int column = 0; column < columnSize; ++column)
        {

            QString key = m_char_vector.at(row).at(column);
            painter.setClipRect(column*m_cellSize, row*m_cellSize, m_cellSize, m_cellSize);

            painter.drawText(column*m_cellSize + (m_cellSize / 2) - fontMetrics.width(key)/2,
                             row*m_cellSize + 4 + fontMetrics.ascent(),
                             key);
        }
    }
}

void CharacterWidget::SetCharacterStr(const QVector<QVector<QString> > inputCharVector)
{
    m_char_vector.clear();
    if(inputCharVector.size() != 0 )
    {
        m_char_vector = inputCharVector;
    }
    else
    {
        return;
    }
    int maxColumn = 0;
    int vectorSize = inputCharVector.size();
    for(int index=0; index<vectorSize; ++index)
    {
        if(inputCharVector.at(index).size() > maxColumn)
        {
            maxColumn = inputCharVector.at(index).size();
        }
    }
    m_columnNum = maxColumn;

    this->setFixedHeight(vectorSize*m_cellSize+2);
    this->setFixedWidth(m_columnNum*m_cellSize+2);
    update();
}

