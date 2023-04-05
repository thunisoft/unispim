#ifndef DPIADAPTOR_H
#define DPIADAPTOR_H

#include <QWidget>
#include <QComboBox>
#include <QCheckBox>
#include <QRadioButton>
#include <QTableWidget>
#include <QTextEdit>
#include <QHeaderView>
#include <QLineEdit>
#include <QLabel>
#include <QPushButton>

class DpiAdaptor : public QWidget
{
    Q_OBJECT
public:
    explicit DpiAdaptor(QWidget *parent = nullptr);

public:
       void ChangeObjectSizeAndFontSize(const QWidget& object, double geoRate,double fontRate);
       void ResetTheStyleSheet(QWidget* currentWidget);

signals:

};

#endif // DPIADAPTOR_H
