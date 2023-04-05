#ifndef CUSTOMIZEQTEXTEDIT_H
#define CUSTOMIZEQTEXTEDIT_H

#include <QTextEdit>

class CustomizeQTextEdit : public QTextEdit
{
public:
    explicit CustomizeQTextEdit(QWidget* parent = 0);

private:
    void Init();

    QString m_style_sheet;
    QString m_style_sheet_of_scrollbar;
};

#endif // CUSTOMIZEQTEXTEDIT_H
