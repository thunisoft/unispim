#include "searchlineedit.h"
#include <QPushButton>
#include <QHBoxLayout>

#if _MSC_VER >= 1600
#pragma execution_character_set("utf-8")
#endif
SearchLineEdit::SearchLineEdit(QWidget *parent)
{
	parent;
    InitWidget();
}

void SearchLineEdit::enterEvent(QEvent *event)
{
    emit isFocused();
}

void SearchLineEdit::leaveEvent(QEvent *event)
{

}

void SearchLineEdit::focusInEvent(QFocusEvent *event)
{
    QLineEdit::focusInEvent(event);
    EditTextChanged();
}

void SearchLineEdit::focusOutEvent(QFocusEvent *event)
{
    QLineEdit::focusOutEvent(event);

    if(this->text().isEmpty())
    {
        m_searchBtn->setVisible(true);
        m_closeBtn->setVisible(false);
    }
    else
    {
        m_searchBtn->setVisible(false);
        m_closeBtn->setVisible(true);
    }
}

void SearchLineEdit::TextChangedSlot()
{
    EditTextChanged();
}

void SearchLineEdit::ClearInputContent()
{
    this->clear();
}

void SearchLineEdit::InitWidget()
{
    QHBoxLayout* layout = new QHBoxLayout();
    m_searchBtn = new QPushButton();
    m_searchBtn->setObjectName("searchBtn");
    m_closeBtn = new QPushButton();
    m_closeBtn->setObjectName("closeBtn");

    m_searchBtn->setCursor(QCursor(Qt::PointingHandCursor));
    m_closeBtn->setCursor(QCursor(Qt::PointingHandCursor));
    layout->addStretch();
    layout->addWidget(m_searchBtn);
    layout->addWidget(m_closeBtn);
    layout->setContentsMargins(0,0,10,0);
    this->setLayout(layout);

    m_closeBtn->setVisible(false);

    this->setTextMargins(10,0,0,0);
    this->setPlaceholderText("请输入搜索内容");

    this->setFixedSize(230,30);
    connect(this, SIGNAL(textChanged(QString)),this,SLOT(TextChangedSlot()));
    connect(m_closeBtn, SIGNAL(clicked()), this, SLOT(ClearInputContent()));
}

void SearchLineEdit::EditTextChanged()
{
    if(this->text().isEmpty())
    {
        m_searchBtn->setVisible(true);
        m_closeBtn->setVisible(false);
    }
    else
    {
        m_searchBtn->setVisible(false);
        m_closeBtn->setVisible(true);
    }
}
