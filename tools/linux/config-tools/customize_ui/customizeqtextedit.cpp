#include "customizeqtextedit.h"
#include <QScrollBar>

CustomizeQTextEdit::CustomizeQTextEdit(QWidget* parent):
                                          QTextEdit(parent)
                                         ,m_style_sheet("")
                                         ,m_style_sheet_of_scrollbar("")
{
    Init();
}

void CustomizeQTextEdit::Init()
{
    m_style_sheet = ""
                    " QTextEdit { "
                    " border : 1px solid rgb(230, 230, 230);"
                    " background : rgb(246, 246, 246);"
                    " font-size:15px; "
                    " } "
                    "";
    m_style_sheet_of_scrollbar = ""
                                 " QScrollArea { "
                                 " border:0px; "
                                 " } "
                                 " QScrollBar:vertical { "
                                 " background : rgba(0, 0, 0, 0%);"
                                 " width : 6px;                   "
                                 " margin : 0px, 0px, 0px, 0px;   "
                                 " } "
                                 " QScrollBar::handle:vertical {"
                                 " width : 6px; "
                                 " background:rgb(191, 191, 191);"
                                 " border-radius : 3px; " // 滚动条两端变成椭圆
                                 //" margin-right : 1px;"
                                 " } "
                                 " QScrollBar::handle:hover {"
                                 " width : 6px; "
                                 " background:rgb(128, 128, 128);"
                                 " border-radius : 3px; "   // 滚动条两端变成椭圆
                                 " } "
                                 " QScrollBar::add-line:vertical { "
                                 " border:none; "
                                 " } "
                                 " QScrollBar::sub-line:vertical { "
                                 " border:none; "
                                 " } "
                                 ""   ;

    this->setStyleSheet(m_style_sheet);
    this->verticalScrollBar()->setStyleSheet(m_style_sheet_of_scrollbar);
}
