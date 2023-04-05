#ifndef CUSTOMIZETABLEWIDGETFORADDWORDLIB_H
#define CUSTOMIZETABLEWIDGETFORADDWORDLIB_H

#include <QTableWidget>

#include <QTableWidgetItem>

class CustomizeTablewidgetForAddWordlib : public  QTableWidget
{
public:
    explicit CustomizeTablewidgetForAddWordlib(QWidget* parent);

private:
    QString m_style_sheet;
    QString m_style_sheet_of_scrollbar;
public:
    inline void publicUpdateGeometries(){

        updateGeometries();
    }
};

#endif // CUSTOMIZETABLEWIDGETFORADDWORDLIB_H
