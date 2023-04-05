#include "spsetplandlg.h"
#include "ui_spsetplandlg.h"
#include "config.h"

#include <QSettings>
#include <QFile>
#include <QTextCodec>
#include <QStringList>
#include <QTextStream>
#include <QList>
#include <QRegExp>
#include <QValidator>
#include <QDesktopServices>
#include <Windows.h>
#include <QFileInfo>
#include <tchar.h>
#include <winnt.h>
#include "iniutils.h"


SPSetPlanDlg::SPSetPlanDlg(QWidget *parent) :
    CustomizeQWidget(parent),
    ui(new Ui::SPSetPlanDlg)
{
    ui->setupUi(this);
    InitWidget();
    ConnectSignalToSlot();
    LoadConfig();
}

SPSetPlanDlg::~SPSetPlanDlg()
{
    delete ui;
}

void SPSetPlanDlg::InitWidget()
{
    this->setProperty("type","borderwidget");
    this->setWindowTitle("双拼方案");
    ui->initial_tab->setColumnCount(2);
    ui->final_tab->setColumnCount(2);
    ui->zeroinitial_tab->setColumnCount(2);
    QStringList header;
    header << "音节" << "按键";

    QRegExp reg("[A-Z]{1,3}");
    ui->key_lineedit->setValidator(new QRegExpValidator(reg));
    ui->key_lineedit->setEnabled(false);
    ui->initial_tab->setHorizontalHeaderLabels(header);
    ui->final_tab->setHorizontalHeaderLabels(header);
    ui->zeroinitial_tab->setHorizontalHeaderLabels(header);

    ui->initial_tab->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->initial_tab->setSelectionMode(QAbstractItemView::SingleSelection);
    ui->initial_tab->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ui->initial_tab->horizontalHeader()->setStretchLastSection(true);
    ui->initial_tab->setColumnWidth(0,60);
    ui->initial_tab->setColumnWidth(1,60);

    ui->final_tab->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->final_tab->setSelectionMode(QAbstractItemView::SingleSelection);
    ui->final_tab->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ui->final_tab->horizontalHeader()->setStretchLastSection(true);
    ui->final_tab->setColumnWidth(0,60);
    ui->final_tab->setColumnWidth(1,60);

    ui->zeroinitial_tab->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->zeroinitial_tab->setSelectionMode(QAbstractItemView::SingleSelection);
    ui->zeroinitial_tab->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ui->zeroinitial_tab->horizontalHeader()->setStretchLastSection(true);
    ui->zeroinitial_tab->setColumnWidth(0, 60);
    ui->zeroinitial_tab->setColumnWidth(1, 60);

    QStringList planNameList;
    planNameList << "华宇双拼"<<"DOS双拼" <<"蓝天双拼"
    << "拼音加加双拼" << "微软双拼" << "智能ABC双拼" <<"中文之星双拼" << "自然码双拼";
    ui->comboBox->addItems(planNameList);
    ui->sp_title_widget->setProperty("type","window_title_widget");
    ui->pushButton->setProperty("type","normal");
    ui->reset_sp_btn->setProperty("type","normal");
    ui->confirm_btn->setProperty("type","normal");
    ui->cancel_btn->setProperty("type","normal");
    ui->label->setProperty("type","h2");
    ui->label_2->setProperty("type","h2");
    ui->label_3->setProperty("type","h2");
    ui->label_4->setProperty("type","h2");
    ui->label_5->setProperty("type","h2");


}

void SPSetPlanDlg::LoadConfig()
{
    QString dirPath = Config::Instance()->GetSPIniFileDir();
    QString fileName;
    QString fullPath = dirPath + fileName;
    QString baseName = QFileInfo(fullPath).baseName();
    int itemIndex = ui->comboBox->findText(baseName);
    ui->comboBox->setCurrentIndex(itemIndex);
    IniParser(fullPath);
}

void SPSetPlanDlg::IniParser(const QString iniFilePath)
{
    //先清空控件
    ui->initial_tab->clearContents();
    ui->final_tab->clearContents();
    ui->zeroinitial_tab->clearContents();

    //先获取有多少个key值
    bool exists = QFile::exists(iniFilePath);
    if(!exists)
    {
        return;
    }

    const TCHAR* iniFilePathChar = reinterpret_cast<const TCHAR*>(iniFilePath.utf16());
    TCHAR szBuffer[MAX_PATH] = {0};
    KEY_NAME* result = new KEY_NAME;
    int initialCharacterNum = 0;
    int initialkeyNum = IniUtils::GetKeyName(initialCharacterNum,szBuffer,result);
    ui->initial_tab->setRowCount(initialkeyNum);
    for(int index=0; index<result->nKeyNum; ++index)
    {
      TCHAR value[256];
      GetPrivateProfileString(TEXT("Initial"),result->szKeyBuf[index],TEXT(""),value,256,iniFilePathChar);
      QString currentValue = QString::fromWCharArray(value);
      QString currentKey = QString::fromWCharArray(result->szKeyBuf[index]);

      QTableWidgetItem* keyitem = new QTableWidgetItem();
      keyitem->setText(currentKey);
      ui->initial_tab->setItem(index,0,keyitem);

      QTableWidgetItem* valueItem = new QTableWidgetItem();
      valueItem->setText(currentValue);
      ui->initial_tab->setItem(index,1,valueItem);
    }

    memset(result,0x00,sizeof(KEY_NAME));
    memset(szBuffer,0x00,MAX_PATH);
    int finalCharacterNum = GetPrivateProfileString(TEXT("Final"),NULL,TEXT(""),szBuffer,sizeof(szBuffer)/sizeof(TCHAR),iniFilePathChar);
    int finalkeyNum = IniUtils::GetKeyName(finalCharacterNum,szBuffer,result);
    ui->final_tab->setRowCount(finalkeyNum);
    for(int index=0; index<result->nKeyNum; ++index)
    {
      TCHAR value[256];
      GetPrivateProfileString(TEXT("Final"),result->szKeyBuf[index],TEXT(""),value,256,iniFilePathChar);
      QString currentValue = QString::fromWCharArray(value);
      QString currentKey = QString::fromWCharArray(result->szKeyBuf[index]);

      QTableWidgetItem* keyitem = new QTableWidgetItem();
      keyitem->setText(currentKey);
      ui->final_tab->setItem(index,0,keyitem);

      QTableWidgetItem* valueItem = new QTableWidgetItem();
      valueItem->setText(currentValue);
      ui->final_tab->setItem(index,1,valueItem);
    }

    memset(result,0x00,sizeof(KEY_NAME));
    memset(szBuffer,0x00,MAX_PATH);
    int ZeroFinalCharacterNum = GetPrivateProfileString(TEXT("ZeroFinal"),NULL,TEXT(""),szBuffer,sizeof(szBuffer)/sizeof(TCHAR),iniFilePathChar);
    int zeroKeyNum = IniUtils::GetKeyName(ZeroFinalCharacterNum,szBuffer,result);
    ui->zeroinitial_tab->setRowCount(zeroKeyNum);
    for(int index=0; index<result->nKeyNum; ++index)
    {
      TCHAR value[256];
      GetPrivateProfileString(TEXT("ZeroFinal"),result->szKeyBuf[index],TEXT(""),value,256,iniFilePathChar);
      QString currentValue = QString::fromWCharArray(value);
      QString currentKey = QString::fromWCharArray(result->szKeyBuf[index]);

      QTableWidgetItem* keyitem = new QTableWidgetItem();
      keyitem->setText(currentKey);
      ui->zeroinitial_tab->setItem(index,0,keyitem);

      QTableWidgetItem* valueItem = new QTableWidgetItem();
      valueItem->setText(currentValue);
      ui->zeroinitial_tab->setItem(index,1,valueItem);
    }
    delete result;
}

void SPSetPlanDlg::ConnectSignalToSlot()
{
    connect(ui->initial_tab,SIGNAL(itemClicked(QTableWidgetItem*)), this, SLOT(SlotShengmuSelected(QTableWidgetItem*)));
    connect(ui->final_tab, SIGNAL(itemClicked(QTableWidgetItem*)), this, SLOT(SlogYunmuSelected(QTableWidgetItem*)));
    connect(ui->zeroinitial_tab, SIGNAL(itemClicked(QTableWidgetItem*)), this, SLOT(SlotZeroShengmuSelected(QTableWidgetItem*)));
    connect(ui->key_lineedit, SIGNAL(textChanged(const QString)), this, SLOT(SlotLineEditChanged(const QString)));
    connect(ui->confirm_btn, SIGNAL(clicked()), this, SLOT(SlotConfirmed()));
    connect(ui->cancel_btn, SIGNAL(clicked()), this, SLOT(SlotCanceled()));
    connect(ui->reset_sp_btn, SIGNAL(clicked()), this, SLOT(SlotResetSPPlan()));
    connect(ui->comboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(SlotSPPlanChanged(int)));

}

void SPSetPlanDlg::SlotShengmuSelected(QTableWidgetItem* item)
{
    Q_UNUSED(item);
    QList<QTableWidgetItem*> itemList = ui->initial_tab->selectedItems();
    int rowNum = itemList.at(0)->row();
    m_selectedItem = ui->initial_tab->item(rowNum,1);
    QString keyStr = m_selectedItem->text();
    ui->key_lineedit->clear();
    ui->key_lineedit->setText(keyStr);
    ui->key_lineedit->setEnabled(true);

}

void SPSetPlanDlg::SlogYunmuSelected(QTableWidgetItem* item)
{
    Q_UNUSED(item);
    QList<QTableWidgetItem*> itemList = ui->final_tab->selectedItems();
    int rowNum = itemList.at(0)->row();
    m_selectedItem = ui->final_tab->item(rowNum,1);
    QString keyStr = m_selectedItem->text();
    ui->key_lineedit->clear();
    ui->key_lineedit->setText(keyStr);
    ui->key_lineedit->setEnabled(true);
}
void SPSetPlanDlg::SlotZeroShengmuSelected(QTableWidgetItem* item)
{
    Q_UNUSED(item);
    QList<QTableWidgetItem*> itemList = ui->zeroinitial_tab->selectedItems();
    int rowNum = itemList.at(0)->row();
    m_selectedItem = ui->zeroinitial_tab->item(rowNum,1);
    QString keyStr = m_selectedItem->text();
    ui->key_lineedit->clear();
    ui->key_lineedit->setText(keyStr);
    ui->key_lineedit->setEnabled(true);
}

void SPSetPlanDlg::SlotConfirmed()
{
    SaveConfigInfo();
    QDialog::accept();
}

void SPSetPlanDlg::SlotCanceled()
{
    QDialog::reject();
}

void SPSetPlanDlg::SlotResetSPPlan()
{
    QString fileName = ui->comboBox->currentText() + ".ini";
    QString fullPath = Config::Instance()->GetSPIniFileDir() + fileName;
    IniParser(fullPath);
}

void SPSetPlanDlg::SlotLineEditChanged(const QString str)
{
    m_selectedItem->setText(str);
}

void SPSetPlanDlg::SlotSPPlanChanged(int index)
{
    QString fileName = ui->comboBox->itemText(index) + ".ini";
    QString fullPath = Config::Instance()->GetSPIniFileDir() + fileName;
    IniParser(fullPath);
}

void SPSetPlanDlg::SaveConfigInfo()
{
    QString fileName = ui->comboBox->currentText() + ".ini";
    QString fullPath = Config::Instance()->GetSPIniFileDir() + fileName;
    int initial_rowNum = ui->initial_tab->rowCount();
    for(int initialIndex = 0; initialIndex<initial_rowNum;++initialIndex)
    {
        QString currentKey = ui->initial_tab->item(initialIndex,0)->text();
        QString currentValue = ui->initial_tab->item(initialIndex,1)->text();
        const TCHAR* currentKeyChar = reinterpret_cast<const wchar_t*>(currentKey.utf16());
        const TCHAR* currentValueChar = reinterpret_cast<const wchar_t*>(currentValue.utf16());
        WritePrivateProfileString(TEXT("Initial"),currentKeyChar, currentValueChar, reinterpret_cast<const wchar_t*>(fullPath.utf16()));
    }

    int final_rowNum = ui->final_tab->rowCount();
    for(int finalIndex = 0; finalIndex<final_rowNum; ++finalIndex)
    {
        QString currentKey = ui->final_tab->item(finalIndex,0)->text();
        QString currentValue = ui->final_tab->item(finalIndex,1)->text();
        const TCHAR* currentKeyChar = reinterpret_cast<const wchar_t*>(currentKey.utf16());
        const TCHAR* currentValueChar = reinterpret_cast<const wchar_t*>(currentValue.utf16());
        WritePrivateProfileString(TEXT("Final"),currentKeyChar, currentValueChar, reinterpret_cast<const wchar_t*>(fullPath.utf16()));
    }

    int zerofinal_rowNum = ui->zeroinitial_tab->rowCount();
    for(int zerofinalIndex = 0; zerofinalIndex<zerofinal_rowNum;++zerofinalIndex)
    {
        QString currentKey = ui->zeroinitial_tab->item(zerofinalIndex,0)->text();
        QString currentValue = ui->zeroinitial_tab->item(zerofinalIndex,1)->text();
        const TCHAR* currentKeyChar = reinterpret_cast<const wchar_t*>(currentKey.utf16());
        const TCHAR* currentValueChar = reinterpret_cast<const wchar_t*>(currentValue.utf16());
        WritePrivateProfileString(TEXT("ZeroFinal"),currentKeyChar, currentValueChar, reinterpret_cast<const wchar_t*>(fullPath.utf16()));
    }

}








