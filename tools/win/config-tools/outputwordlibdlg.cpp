#include "outputwordlibdlg.h"
#include "ui_outputwordlibdlg.h"
#include "customize_ui/wordlibitemwidget.h"
#include <QListView>
#include <QFileDialog>
#include <QStandardPaths>
#include <QGridLayout>
#include <math.h>
#include "config.h"
#include "dpiadaptor.h"
#include "utils.h"
OutputWordlibDlg::OutputWordlibDlg(QWidget *parent) :
    CustomizeQWidget(parent),
    ui(new Ui::OutputWordlibDlg)
{
    ui->setupUi(this);
    InitWidget();
}

OutputWordlibDlg::~OutputWordlibDlg()
{
    delete ui;
}

void OutputWordlibDlg::InitWidget()
{

    this->setAttribute(Qt::WA_TranslucentBackground,true);
    ui->output_wordlib_title_widget->setProperty("type","window_title_widget");
    ui->getOutputDirBtn->setProperty("type","normal");
    ui->output_btn->setProperty("type","normal");
    ui->cancel_btn->setProperty("type","normal");
    ui->label->setProperty("type","h2");
    ui->label_2->setProperty("type","h2");
    ui->label_3->setProperty("type","h2");
    this->setProperty("type","borderwidget");
    ui->comboBox->setProperty("type","expandingcombo");
    ui->lineEdit->setProperty("type","NormalLineEdit");

    ui->tableWidget->clear();
    ui->tableWidget->horizontalHeader()->setVisible(false);
    ui->tableWidget->verticalHeader()->setVisible(false);
    ui->tableWidget->setColumnCount(3);
    ui->tableWidget->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    ui->tableWidget->setShowGrid(false);
    ui->comboBox->setView(new QListView());
    ui->tableWidget->setStyleSheet("QTableWidget{border:none;}");
    ui->tableWidget->setSelectionMode(QTableView::NoSelection);
    ui->tableWidget->setFocusPolicy(Qt::NoFocus);
    ui->tableWidget->setProperty("type","nobordertable");
    QStringList itemLists;
    itemLists << "文本文件(.txt)";
    itemLists << "词库文件(.uwl)";
    ui->comboBox->addItems(itemLists);
    connect(ui->cancel_btn,SIGNAL(clicked()), this,SLOT(close()));
    connect(ui->getOutputDirBtn,SIGNAL(clicked()), this, SLOT(SlotsGetSaveFileDir()));
    connect(ui->cancel_btn, SIGNAL(clicked()),this,SLOT(OnCancelButtonClicked()));
    connect(ui->output_btn, SIGNAL(clicked()),this,SLOT(OnOkButtonClicked()));
}

void OutputWordlibDlg::SlotsGetSaveFileDir()
{
    QString fileDir = QFileDialog::getExistingDirectory(this,"选择要保存的地址",QStandardPaths::writableLocation(QStandardPaths::DesktopLocation));
    if(!fileDir.isEmpty())
    {
        ui->lineEdit->setText(fileDir);
    }
   
}

 void OutputWordlibDlg::SetWordlibVector(QVector<QString> wordlibVector)
 {
    if(wordlibVector.size() == 0)
    {
        return;
    }
    m_itemNameVector.clear();
    m_itemNameVector = wordlibVector;
    UpdateShow();
 }

 void OutputWordlibDlg::OnOkButtonClicked()
 {
     QString output_path = ui->lineEdit->text();
     QDir dir(output_path);
     if(output_path.isEmpty())
     {
         Utils::NoticeMsgBox("输出路径不能为空",this);
     }
     else if(!dir.exists())
     {
         Utils::NoticeMsgBox("输出路径不存在\n请重新选择",this);
     }
     else
     {
     QDialog::accept();
     }
 }

 void OutputWordlibDlg::OnCancelButtonClicked()
 {
     QDialog::reject();
 }

 void OutputWordlibDlg::RefreshWordlibVector(QString inputItem)
 {
     if(m_itemNameVector.contains(inputItem))
     {
         m_itemNameVector.removeOne(inputItem);
     }
     UpdateShow();
 }

 void OutputWordlibDlg::UpdateShow()
 {
     ui->tableWidget->setRowCount(0);
     int rowNum = ceil(m_itemNameVector.size()/3.0);
     ui->tableWidget->setRowCount(rowNum);

     for(int index=1; index<=m_itemNameVector.size(); ++index)
     {
         int currentRow = ceil(index/3.0);
         int currentColumn = index - (currentRow-1)*3;
         WordlibItemWidget* item = new WordlibItemWidget(m_itemNameVector.at(index-1));
         connect(item, SIGNAL(ItemDelete(QString)), this, SLOT(RefreshWordlibVector(QString)));

         double geoRate = Config::Instance()->GetGeoRate();

         int newHeight = 30*geoRate;
         item->resize(80*geoRate,25*geoRate);
         item->RefreshTextShow();
         ui->tableWidget->setRowHeight(currentRow-1, newHeight+5);

         ui->tableWidget->setCellWidget(currentRow-1,currentColumn-1,item);
     }
 }

 QString OutputWordlibDlg::GetSaveDirPath()
 {
     QString fileDir = ui->lineEdit->text();
     return fileDir;
 }

 QString OutputWordlibDlg::GetOutputFileType()
 {
    return ui->comboBox->currentText();
 }

