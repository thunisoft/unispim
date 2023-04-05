#include "msgboxutils.h"

#include "customize_ui/customizemessageboxdialog.h"
#include "customize_ui/customizemessageboxwithreturnvaluedialog.h"

MsgBoxUtils::MsgBoxUtils()
{

}

int MsgBoxUtils::NoticeMsgBox(const QString& notice_content, QString title, bool isReturnValue, QWidget* parent)
{
      CustomizeMessageBoxDialog dialog(notice_content);
      dialog.SetTitle(title);
      dialog.EnableWithReturnValue(isReturnValue);
      if(dialog.exec() == QDialog::Rejected)
          return 0;
      return 1;
}

int MsgBoxUtils::NoticeMsgBoxWithReturnValue(const QString& notice_content, QWidget* parent)
{

    CustomizeMessageBoxWithReturnValueDialog *dialog = new CustomizeMessageBoxWithReturnValueDialog(notice_content, parent);
    if(dialog->exec() == QDialog::Rejected)
        return 0;
    return 1;
}
