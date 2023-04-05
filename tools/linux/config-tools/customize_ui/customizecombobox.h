#ifndef CUSTOMIZECOMBOBOX_H
#define CUSTOMIZECOMBOBOX_H
#include <QComboBox>
#include <QWheelEvent>
#include <QFocusEvent>

class CustomizeCombobox : public QComboBox
{
    Q_OBJECT
public:
    CustomizeCombobox(QWidget*parent=0);
    ~CustomizeCombobox();

signals:
    void focusOut();

private:
    void Init();

protected:
    void wheelEvent(QWheelEvent *e);

    void focusOutEvent(QFocusEvent *e) Q_DECL_OVERRIDE;

private:
    QString m_style_sheet;
};

#endif // CUSTOMIZECOMBOBOX_H
