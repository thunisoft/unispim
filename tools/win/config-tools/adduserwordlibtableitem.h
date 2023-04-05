#ifndef ADDUSERWORDLIBTABLEITEM_H
#define ADDUSERWORDLIBTABLEITEM_H

#include <QWidget>

namespace Ui {
class AddUserWordlibTableItem;
}

class AddUserWordlibTableItem : public QWidget
{
    Q_OBJECT

public:
    explicit AddUserWordlibTableItem(QWidget *parent = 0);
    ~AddUserWordlibTableItem();

private:
    Ui::AddUserWordlibTableItem *ui;
signals:
    void import_user_wordlib_btn_clicked(bool);

};

#endif // ADDUSERWORDLIBTABLEITEM_H
