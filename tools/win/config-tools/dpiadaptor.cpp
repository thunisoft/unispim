#include "dpiadaptor.h"
#include <QDir>
#include <QApplication>
#include <QElapsedTimer>
#include <QDebug>
#include "element_style.h"

DpiAdaptor::DpiAdaptor(QWidget *parent) : QWidget(parent)
{

}

void DpiAdaptor::ResetTheStyleSheet(QWidget *currentWidget)
{
    QString filePath = QDir::toNativeSeparators(QApplication::applicationDirPath() + "\\mainwindow.qss");

    QFile file(filePath);

    if(file.open(QIODevice::ReadOnly))
    {
        QString fileContent = file.readAll();
        currentWidget->setStyleSheet(fileContent);
        file.close();
    }
}
void DpiAdaptor:: ChangeObjectSizeAndFontSize(const QWidget& object, double geoRate,double fontRate)
{
    //修改对应的子类的字体大小
    QList<QComboBox*> resultCombo = object.findChildren<QComboBox*>();
    for(int index=0; index<resultCombo.size(); ++index)
    {
        if(resultCombo.at(index)->property("type") != "expandingcombo")
        {
            int comboWidth = 80*geoRate;
            int comboHeight = 30*geoRate;
            resultCombo.at(index)->setFixedSize(comboWidth,comboHeight);
        }
        else
        {
            int comboWidth = 100*geoRate;
            int comboHeight = 30*geoRate;
            resultCombo.at(index)->setFixedSize(comboWidth,comboHeight);
        }


        QFont font = resultCombo.at(index)->font();
        font.setPixelSize(14*fontRate);
        resultCombo.at(index)->setFont(font);
        resultCombo.at(index)->view()->setFont(font);

    }
    QList<QCheckBox*> resultCheck = object.findChildren<QCheckBox*>();
    for(int index=0; index<resultCheck.size(); ++index)
    {
        int checkWidth = static_cast<int>(16*geoRate);
        int checkHeight = static_cast<int>(16*geoRate);
        int fontSize = static_cast<int>(14*fontRate);
        resultCheck.at(index)->setStyleSheet(QString("QCheckBox{font-size:%1px;}QCheckBox::indicator{width:%2px;height:%3px;}").arg(fontSize).arg(checkWidth).arg(checkHeight));
    }
    QList<QRadioButton*> resultRadio = object.findChildren<QRadioButton*>();
    for(int index=0; index<resultRadio.size(); ++index)
    {
        int radioWidth = static_cast<int>(16*geoRate);
        int radioHeight = static_cast<int>(16*geoRate);
        int fontSize = static_cast<int>(14*fontRate);
        resultRadio.at(index)->setStyleSheet(QString("QRadioButton{font-size:%1px;}QRadioButton::indicator{width:%2px;height:%3px;}").arg(fontSize).arg(radioWidth).arg(radioHeight));
    }
    QList<QTableWidget*> resultTable = object.findChildren<QTableWidget*>();
    for(int index=0; index<resultTable.size(); ++index)
    {
        QTableWidget* currentTable = resultTable.at(index);
        int indicator = static_cast<int>(14*geoRate);
        int fontSize = static_cast<int>(14*fontRate);
        currentTable->setStyleSheet(QString("QTableWidget::indicator{width:%1px;height:%2px;} QTableWidget{font-size:%3px;} QTableWidget::item{border:none;}").arg(indicator).arg(indicator).arg(fontSize));

        int rowHeight = static_cast<int>(45*geoRate);
        currentTable->verticalHeader()->setDefaultSectionSize(rowHeight);
        int HeaderWidth = static_cast<int>(30*geoRate);
        currentTable->horizontalHeader()->setStyleSheet(QString("QHeaderView{height:%1px;font-size:%2px;}").arg(HeaderWidth).arg(fontSize));


        currentTable->setColumnWidth(0,130*geoRate);
        currentTable->setColumnWidth(1,130*geoRate);
        currentTable->setColumnWidth(2,80*geoRate);
        currentTable->setColumnWidth(3,60*geoRate);
        currentTable->setColumnWidth(4,80*geoRate);

    }
    QList<QLabel*> resultLabel = object.findChildren<QLabel*>();
    for(int index=0; index<resultLabel.size(); ++index)
    {
        QString typeName = resultLabel.at(index)->property("type").toString();
        QLabel* currentLabel = resultLabel.at(index);
        QFont font = currentLabel->font();
        if(typeName == "h1")
        {
            int pixSize = static_cast<int>(18*fontRate);
            currentLabel->setStyleSheet(QString("QLabel{font-size:%1px;}").arg(pixSize));
        }
        else if(typeName == "h2")
        {
            font.setPixelSize(static_cast<int>(15*fontRate));
            int pixSize = static_cast<int>(15*fontRate);
            currentLabel->setFont(font);
            currentLabel->setStyleSheet(QString("QLabel{font-size:%1px;}").arg(pixSize));
        }
        else if(typeName == "main_title_label")
        {
            int pixSize = static_cast<int>(16*fontRate);
            currentLabel->setStyleSheet(QString("QLabel{font-size:%1px;}").arg(pixSize));
        }
        else if(typeName == "main_logo_label")
        {
            int width = static_cast<int>(20*geoRate);
            int height = static_cast<int>(20*geoRate);
            currentLabel->setStyleSheet(QString("QLabel{width:%1px;height:%2px;}").arg(width).arg(height));
        }
        else if(typeName == "user_logo_label")
        {
            int width = static_cast<int>(80*geoRate);
            int height = static_cast<int>(85*geoRate);
            currentLabel->setFixedSize(width,height);
        }
        else if(typeName == "greenLabel")
        {
            int pixSize = static_cast<int>(22*fontRate);
            currentLabel->setStyleSheet(QString("QLabel{font-size:%1px;}").arg(pixSize));
        }
        else if(typeName == "bluelabel")
        {
            int pixSize = static_cast<int>(14*fontRate);
            currentLabel->setStyleSheet(QString("QLabel{font-size:%1px;}").arg(pixSize));
            QFont font = currentLabel->font();
            font.setPixelSize(pixSize);
            currentLabel->setFont(font);
        }
        else if(typeName == "aboutLogoLabel")
        {
            int width = static_cast<int>(16*geoRate);
            int height = static_cast<int>(16*geoRate);
            currentLabel->setFixedSize(width,height);
        }
        else if(typeName == "copyright")
        {
            int pixSize = static_cast<int>(12*fontRate);
            currentLabel->setStyleSheet(QString("QLabel{font-size:%1px;}").arg(pixSize));
        }
        else
        {
            if(currentLabel->parent()->property("type").toString() == "window_title_widget")
            {
                font.setPixelSize(16*fontRate);
            }
            else
            {
                font.setPixelSize(14*fontRate);
            }

            currentLabel->setFont(font);
        }
    }
    QList<QPushButton*> resultBtn = object.findChildren<QPushButton*>();
    for(int index=0; index<resultBtn.size(); ++index)
    {
        QPushButton* currentBtn = resultBtn.at(index);
        QString typeName = currentBtn->property("type").toString();
        QFont font = currentBtn->font();
        QStringList normalGroup;
        normalGroup << "normal" << "puretextbtn" << "bordernormal" << "normal-radius-set-btn" << "normalfeedback" << "special-normal";

        if(typeName == "sidebtn")
        {
            int pixSize = static_cast<int>(14*fontRate);
            int sidebtnHeight = static_cast<int>(44*geoRate);
            int paddingLeft = static_cast<int>(40*geoRate);
            //int sidebtnWidth = 150*geoRate;

            currentBtn->setStyleSheet(QString("QPushButton{font-size:%1px;height:%2px;padding-left:%3px;}").arg(pixSize).arg(sidebtnHeight).arg(paddingLeft));

        }
        else if(normalGroup.contains(typeName))
        {
            int pixSize = 14*fontRate;
            int btnWidth = 80*geoRate;
            int btnHeight = 27*geoRate;
            currentBtn->setStyleSheet(QString("QPushButton{font-size:%1px;width:%2px;height:%3px;}").arg(pixSize).arg(btnWidth).arg(btnHeight));
        }
        else if(typeName == "normal-set-btn")
        {
            int pixSize = 14*fontRate;
            int btnWidth = 120*geoRate;
            int btnHeight = 27*geoRate;
            currentBtn->setStyleSheet(QString("QPushButton{font-size:%1px;width:%2px;height:%3px;}").arg(pixSize).arg(btnWidth).arg(btnHeight));
        }
        else if(typeName == "tip")
        {
            int btnWidth = 16*geoRate;
            int btnHeight = 16*geoRate;
            currentBtn->setStyleSheet(QString("QPushButton{width:%1px;height:%2px;}").arg(btnWidth).arg(btnHeight));
        }
        else if(typeName == "closebtn" || typeName == "minbtn")
        {
            int btnWidth = 50*geoRate;
            int btnHeight = 50*geoRate;
            currentBtn->setStyleSheet(QString("QPushButton{width:%1px;height:%2px;}").arg(btnWidth).arg(btnHeight));
        }
        else if(typeName == "phraseOptionBtn")
        {
            int pixSize = 14*fontRate;
            int btnWidth = 40*geoRate;
            int btnHeight = 20*geoRate;
            currentBtn->setStyleSheet(QString("QPushButton{font-size:%1px;width:%2px;height:%3px;}").arg(pixSize).arg(btnWidth).arg(btnHeight));
        }
        else if((typeName == "loginmodebtn") ||(typeName == "Verification_Code"))
        {
            int pixSize = 14*fontRate;
            int height = 22*geoRate;
            currentBtn->setStyleSheet(QString("QPushButton{font-size:%1px;height:%2px;}").arg(pixSize).arg(height));
        }
        else if(typeName == "login")
        {
            int pixSize = 14*fontRate;
            int height = 22*geoRate;
            currentBtn->setStyleSheet(QString("QPushButton{font-size:%1px;height:%2px;}").arg(pixSize).arg(height));
        }
        else if(typeName == "btngroup")
        {
            int btnWidth = 20*geoRate;
            int btnHeight = 20*geoRate;
            currentBtn->setStyleSheet(QString("QPushButton{width:%1px;height:%2px;}").arg(btnWidth).arg(btnHeight));
        }
        else if(typeName == "blueclose")
        {
            int btnWidth = 10*geoRate;
            int btnHeight = 10*geoRate;
            currentBtn->setStyleSheet(QString("QPushButton{width:%1px;height:%2px;}").arg(btnWidth).arg(btnHeight));
        }
        else if(typeName == "loginmodebtn")
        {
            int btnHeight = 44*geoRate;
            currentBtn->setStyleSheet(QString("QPushButton{width:%1px;}").arg(btnHeight));
        }
        QString objectName = currentBtn->objectName();
        QStringList objectNameList;
        objectNameList << "download_more_wordlib" << "input_wordlib_btn" << "output_wordlib_btn" << "create_wordlib_btn" << "insertImageBtn";
        if(objectNameList.contains(objectName))
        {
            int pixSize = 14*fontRate;
            int iconSize = 13*fontRate;
            currentBtn->setStyleSheet(QString("QPushButton{font-size:%1px;qproperty-iconSize:%2px %3px;}").arg(pixSize).arg(iconSize).arg(iconSize));
        }

    }

    QList<QLineEdit*> resultLineEdit = object.findChildren<QLineEdit*>();
    for(int index=0; index<resultLineEdit.size(); ++index)
    {

        QLineEdit* currentLineEdit = resultLineEdit.at(index);
        if(currentLineEdit->property("type").toString() == "NormalLineEdit")
        {
            int fontPixsize = 14*fontRate;
            int height = 27*geoRate;
            currentLineEdit->setStyleSheet(QString("QLineEdit{font-size:%1px;height:%2px;}").arg(fontPixsize).arg(height));


        }
        else
        {
            int fontPixsize = 14*fontRate;
            currentLineEdit->setStyleSheet(QString("QLineEdit{font-size:%1px;}").arg(fontPixsize));
            int height = 27*geoRate;
            int width = currentLineEdit->width();
            currentLineEdit->resize(width,height);
        }



    }
    QList<QTabWidget*> resultTab = object.findChildren<QTabWidget*>();
   for(int index=0; index<resultTab.size(); ++index)
   {
       QTabWidget* currentTab = resultTab.at(index);
       QFont font = currentTab->font();
       font.setPixelSize(14*fontRate);
       currentTab->tabBar()->setFont(font);
   }

   QList<QWidget*> resultWidget = object.findChildren<QWidget*>();
   for(int index=0; index<resultWidget.size(); ++index)
   {
       QWidget* pWidget = resultWidget.at(index);
       int width = pWidget->minimumWidth();
       int height = pWidget->minimumHeight();
       if((width !=0)&& (height != 0))
       {
           pWidget->resize(width*geoRate,height*geoRate);
       }


   }
   QList<QTextEdit*> resultTextEdit = object.findChildren<QTextEdit*>();
   for(int index=0; index<resultTextEdit.size(); ++index)
   {
        QFont font = resultTextEdit.at(index)->font();
        font.setPixelSize(14*fontRate);
        resultTextEdit.at(index)->setFont(font);
        int pixSize = 14*fontRate;
        resultTextEdit.at(index)->setStyleSheet(QString("QTextEdit{font-size:%1px;}").arg(pixSize));
   }

}
