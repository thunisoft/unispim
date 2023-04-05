#ifndef SEARCHLINEEDIT_H
#define SEARCHLINEEDIT_H

#include <QLineEdit>
#include <QWidget>
#include <QEnterEvent>
#include <QPushButton>


class SearchLineEdit : public QLineEdit
{

    Q_OBJECT
public:
    SearchLineEdit(QWidget*parent = nullptr);

signals:
    void searchForString(QString searchString);
    void closeSearch();
    void isFocused();

protected:
    void enterEvent(QEvent *event) override;
    void leaveEvent(QEvent *event) override;

    void focusInEvent(QFocusEvent* event) override;
    void focusOutEvent(QFocusEvent* event) override;


private slots:
    void TextChangedSlot();
    void ClearInputContent();
private:
    void InitWidget();
    void EditTextChanged();

private:
    QPushButton* m_closeBtn;
    QPushButton* m_searchBtn;
};

#endif // SEARCHLINEEDIT_H
