#include <QWidget>
#include <QPainter>
#include <QToolTip>
#include "characterwidget.h"
#include <QGridLayout>
#include <QPushButton>
#include <QFontDatabase>
#include <QApplication>
#include "characterparser.h"

#if _MSC_VER >= 1600
#pragma execution_character_set("utf-8")
#endif

CharacterWidget::CharacterWidget(QWidget *parent)
    : QWidget(parent),
      m_columnNum(12)
{
    m_cellSize = 35.0;
    setMouseTracking(true);
    updateFont(QFont(QString::fromLocal8Bit("微软雅黑")));
    updateSize(QString("12"));
    this->setFocusPolicy(Qt::NoFocus);
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
    int row = static_cast<int>(widgetPosition.y()/m_cellSize);
    int column = static_cast<int>(widgetPosition.x()/m_cellSize);
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


    m_hoverKey = key;
    m_hoverRow = row;
    m_hoverColumn = column;

    SetItemSelected(row,column);
    QString tab_name = this->property("tabtype").toString();
    QString fontFamily = GetTabFontName(tab_name).family();

    bool isOK = false;
    QString singlefontFamily = GetFontNameBySymbol(key,isOK).family();
    if(isOK)
    {
        fontFamily = singlefontFamily;
    }

    QString text =
            QString("<html><style>p{ margin:10px 0px 0px 0px;}</style><body>")+
            QString("<p align=\"center\"><span style=\"font-size: 32pt; font-family: %1\">").arg(fontFamily)
                  + key
                  + QString("</span><p align=\"center\">%1</p></p>").arg(CharacterParser::Instance()->GetSymbolDescribe(tab_name,key))
                  + QString("</body></html>");

    QPoint toolTipPos((column+1)*m_cellSize, (row+0.5)*m_cellSize);
    QToolTip::showText(this->mapToGlobal(toolTipPos), text, this);

    update();
}

void CharacterWidget::mouseReleaseEvent(QMouseEvent *event)
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
    event->accept();
}

void CharacterWidget::paintEvent(QPaintEvent *event)
{
    QPainter painter(this);
    QString widghtType = this->property("type").toString();
    painter.fillRect(event->rect(), QBrush(Qt::white));
    if(widghtType == "recent_header")
    {
        painter.fillRect(event->rect(), QBrush(QColor("#E6E6E6")));
    }


    painter.setFont(displayFont);

    QRect redrawRect = this->rect();
    int top = redrawRect.top();
    int left = redrawRect.left();

    int rows = m_char_vector.size();


    //再在背景框里面画文字
   // QFontMetrics fontMetrics(displayFont);
    painter.setPen(QPen(Qt::black));

    QString tab_name = this->property("tabtype").toString();


    for (int row = 0; row < rows; ++row)
    {
        int columnSize = m_char_vector.at(row).length();
        if(columnSize == 0)
        {
            return;
        }
        for (int column = 0; column < columnSize; ++column)
        {
            painter.setPen(QPen(QColor("#CCCCCC")));
            painter.drawRect(left + column*m_cellSize, top + row*m_cellSize, m_cellSize, m_cellSize);

            painter.setPen(QPen(Qt::black));
            QString key = m_char_vector.at(row).at(column);

            QFont font = GetTabFontName(tab_name);
            font.setPixelSize(16);
            QFontMetrics fontMetrics(font);
            painter.setFont(font);
            QString fontFamily = font.family();

            bool isOK = false;
            QFont singleSymbolFont = GetFontNameBySymbol(key,isOK);
            singleSymbolFont.setPixelSize(16);
            if(isOK)
            {
                painter.setFont(singleSymbolFont);
            }

            //painter.setClipRect(column*m_cellSize, row*m_cellSize, m_cellSize, m_cellSize);

            if ((m_hoverRow == row)&&(m_hoverColumn == column))
                painter.fillRect(column*m_cellSize + 1, row*m_cellSize + 1, m_cellSize, m_cellSize, QBrush(QColor("#C5C5C5")));

            painter.drawText(column*m_cellSize + (m_cellSize / 2) - fontMetrics.width(key)/2 - 2,
                             row*m_cellSize + 8 + fontMetrics.ascent(),
                             key);
        }
    }
}

void CharacterWidget::mousePressEvent(QMouseEvent *event)
{
    event->accept();
}

void CharacterWidget::enterEvent(QEvent *event)
{
    m_hoverKey.clear();
    m_hoverRow = -1;
    m_hoverColumn = -1;
    update();
    event->accept();
}

void CharacterWidget::leaveEvent(QEvent *event)
{
    m_hoverKey.clear();

    m_hoverRow = -1;
    m_hoverColumn = -1;
    update();
    event->accept();
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

void CharacterWidget::ResetTheSymbol()
{
    int rows = m_char_vector.size();

    QGridLayout* mainLayout = new QGridLayout(this);
    mainLayout->setHorizontalSpacing(0);
    mainLayout->setVerticalSpacing(0);
    mainLayout->setMargin(0);

    for (int row = 0; row < rows; ++row)
    {
        int columnSize = m_char_vector.at(row).length();
        if(columnSize == 0)
        {
            return;
        }

        QString tab_name = this->property("tabtype").toString();
        QString fontFamily = GetTabFontName(tab_name).family();

        for (int column = 0; column < columnSize; ++column)
        {
            QPushButton* tempBtn = new QPushButton(this);

            QString key = m_char_vector.at(row).at(column);
            tempBtn->setProperty("type","symbolitem");
            tempBtn->setFixedSize((int)m_cellSize,(int)m_cellSize);
            tempBtn->setText(key);
            QString text = QString("<p>字符: <span style=\"font-size: 24pt; font-family: %1\">").arg(fontFamily)
                          + key;
            tempBtn->setToolTip(text);
            mainLayout->addWidget(tempBtn,row,column,Qt::AlignLeft);
        }
    }
}

void CharacterWidget::SetItemSelected(int row, int column)
{
    if(row<0 || column<0)
    {
        return;
    }

    QPainter painter(this);
    painter.setFont(displayFont);

    QRect redrawRect = this->rect();
    int top = redrawRect.top();
    int left = redrawRect.left();

    //int rows = m_char_vector.size();
    painter.setPen(QColor(255,0,0));
    painter.drawRect(left + column*m_cellSize, top + row*m_cellSize, m_cellSize, m_cellSize);

    //再在背景框里面画文字
    QFontMetrics fontMetrics(displayFont);
    painter.setPen(QPen(Qt::black));

    QString tab_name = this->property("tabtype").toString();
    QFont font = GetTabFontName(tab_name);
    painter.setFont(font);

    QString key = m_char_vector.at(row).at(column);
    painter.setClipRect(column*m_cellSize, row*m_cellSize, m_cellSize, m_cellSize);

    painter.drawText(column*m_cellSize + (m_cellSize / 2) - fontMetrics.width(key)/2 - 2,
                     row*m_cellSize + 8 + fontMetrics.ascent(),
                     key);
}

QFont CharacterWidget::GetTabFontName(QString tab_name)
{
    m_tabFontMap["数字序号"] = QFont("Calibri",14);
    m_tabFontMap["希腊拉丁"] = QFont("SimSun",14);
    m_tabFontMap["英文音符"] = QFont("Times New Roman",14);
    m_tabFontMap["拼音注音"] = QFont("Courier New",14);
    m_tabFontMap["数字单位"] = QFont("SimSun",14);
    m_tabFontMap["中文部首"] = QFont("Microsoft YaHei",14);
    m_tabFontMap["标点符号"] = QFont("Segoe UI Symbol",14);

    if(m_tabFontMap.contains(tab_name))
    {
        return m_tabFontMap[tab_name];
    }
    return QFont("Segoe UI Symbol",14);
}

QFont CharacterWidget::GetFontNameBySymbol(QString symbol,bool &isContain)
{
    m_symbolFontMap["☝"] = QFont("Arial",14);
    m_symbolFontMap["☟"] = QFont("Arial",14);
    m_symbolFontMap["☜"] = QFont("Arial",14);
    m_symbolFontMap["☞"] = QFont("Arial",14);
    m_symbolFontMap["-"] = QFont("Arial",14);
    m_symbolFontMap["∥"] = QFont("Arial",14);
    m_symbolFontMap["￣"] = QFont("Microsoft YaHei",14);
    m_symbolFontMap["⺝"] = QFont("Microsoft YaHei",14);//月字旁
    m_symbolFontMap["⻗"] = QFont("Microsoft YaHei",14);//雨字头
    m_symbolFontMap["㇏"] = QFont("Microsoft YaHei",14);//撇
    m_symbolFontMap["㇀"] = QFont("Microsoft YaHei",14);//捺
    m_symbolFontMap["⺆"] = QFont("Microsoft YaHei",14);
    m_symbolFontMap["㉿"] = QFont("Microsoft YaHei",14);
    m_symbolFontMap["⺆"] = QFont("Microsoft YaHei",14);//同字框

    if(m_symbolFontMap.contains(symbol))
    {
        isContain = true;
        return m_symbolFontMap[symbol];
    }
    isContain = false;

    return QFont("Segoe UI Symbol",14);
}


