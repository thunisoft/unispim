#ifndef IMPORTWORDLIBTEMPLATEDLG_H
#define IMPORTWORDLIBTEMPLATEDLG_H

#include <QWidget>
#include <QDialog>
#include "customize_ui/customize_qwidget.h"

namespace Ui {
class ImportWordlibTemplateDlg;
}

class ImportWordlibTemplateDlg : public CustomizeQWidget
{
    Q_OBJECT

public:
    explicit ImportWordlibTemplateDlg(QWidget *parent = 0);
    ~ImportWordlibTemplateDlg();

public slots:
    void OnConfirmBtnClicked();
    void SetTemplateText(QString inputContent);

private:
    Ui::ImportWordlibTemplateDlg *ui;
};

#endif // IMPORTWORDLIBTEMPLATEDLG_H
